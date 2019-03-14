/*
  PasswordPump_13.ino

  Project Name: PasswordPump, a password manager
  Version:      1.0
  Date:         2018/09/22 - 2019/02/14
  Device:       Arduino Pro Micro w/ ATmega32u4
  Language:     C
  Memory:       32kB flash, 2.5kB SRAM, 1kB EEPROM
  EEprom:       internal=1kB, 25LC256 external=32kB 
  Clock Speed:  16MHz
  Voltage:      5v
  Author:       Daniel J. Murphy
  Components:   RGB LED, 128x32 LED display, one momentaty push button, one 
                rotary encoder, 2 4.7kohm resistors for I2C, 3 220ohm resistors
                for the RGB, 2 25LC256 external EEprom chips
  Purpose
  =======
  - To manage encrypted usernames and passwords and to type them in via 
    keyboard/USB.  To generate and store complex passwords for the user's 
    accounts.

  Features
  ========
  - Authenticate with master password
  - Search for accounts
    - Send account name, username and password as if typed in keyboard
    - Add account name, username, password (generated or not)
    - Edit existing account name, username, password
    - Delete account
  - Data entry via rotary encoder or keyboard and serial monitor
  - Accounts added in alphabetical order
  - Store up to 254 sets of credentials
  - Backup all accounts to a text file
  - Backup all accounts to another external EEprom
  - Logout / de-authenticate via menu
  - Factory reset via menu (when authenticated)
  - Factory reset after 10 failed login attempts
  - Configurable password display on or off
  - Works w/ EEprom internal to AtMega32u4 (1kB) and 25LC256 (32kB). 
  - all passwords (except master password) are encrypted w/ AES128; master 
    password is hashed w/ SHA256.  Master password and all account passwords
    are salted.
  
  Known Defects
  =============  
    - = outstanding
    ? = fixed but needs testing
    * = fixed
  - When \e is embedded in an account name (or username or pw), it is
    interpreted as the ESC character, and the input arrives empty. e.g. 
    INSIGHTORADB\entmetrics.  Only an issue when input via keyboard, not encoder
  - DisplayLine2 needs to be blanked out after retuning from Find or Add acct.
  - Added account 'Add Account' and then deleted, corrupted linked list
  - Added account to the end of the linked list, corrupted linked list
  - When logging in via Remote Desktop and suppling the username and password,
    we seem to 'hit return' after entering just the username.
  - When entering an account name 29 chars long via keyboard, nothing gets 
    entered.
  - automatic initialization after 10 failed logon attempts is prompting the 
    user to confirm the action.
  - test UUID generation
  - FixCorruption leaves the user hung without any accounts to find
  - After deleting account change the location of the menu
  - in the switch statement for EVENT_SINGLE_CLICK the case statements 
    are not in order. When they are in order it doesn't evaluate 
    correctly.
  - can't seem to send a <tab> character via the Keyboard.  Tried KEY_TAB, 
    TAB_KEY, 0x2b, 0xB3, '  '.
  ? single character usernames and passwords are not working well
  ? we are authenticated after blowing all the creds away after 10 failures
  ? The linked list is occasionally becoming corrupt. Added the ability to 
    fix a corrupt linked list. Exact conditions of corruption unknown at this 
    point.
  ? we are only encrypting the first 16 characters of account name, username and
    password.  The sha256 blocksize is 16.
  ? single click after Reset brings you to alpha edit mode
  * Renaming an account in place corrupts the linked list; the account makes
    copies of itself all over the place (a loop in the linked list).  
    Workaround: add a new set of creds and delete the old set of creds.
  * Should probably remove Keyboard ON/OFF from saved properties and always 
    default to Keyboard OFF; or make sure it is always OFF when backing up 
    EEProm.
  * Delete screws up the account count when it leaves a hole.  e.g. add AAA, 
    BBB, CCC; delete BBB, you'll only be able to "Find" AAA.
  * after deleting an account we're returned to the send cred menu instead of  
    the find account menu
  * When you select add account and immediatly scroll through the options there
    is apparent corruption in the menu selections
  * Long clicking out of confirm account delete brings you to the main menu 
    instead of the edit creds menu.
  * When we first enter the find account menu the currentAccount can be wrong
  * don't show Master Password menu choice after authenticating
  * Deleting the head or tail results in corruption
  * When using extended memory crash at line 1367 where the LCD output is 
    trashed. Suspect some conflict w/ the 25LC256 EEprom chip.  Hangs subsequent
    to line 1367. Search for "trashed".
  * Backup All to a file isn't in alphabetical order
  * Backup all isn't consistently printing carriage returns.
  * passwords are not generating correctly
  * the LCD's backlight is not turning on, needed to operate in the dark
  * after add account the account isn't showing in find account
  * after hitting reset show password = 0
  * turning the rotary encoder fast doesn't scroll thru the menu fast [sending 
    output to the serial monitor corrects this] 
  * can't add a new account; can't scroll edit menu on first account add
  * long click seems too long
  * first account added doesn't immediatly show in find account after it's 
    added.
  * after reset, stuck in reset.
  * in LCD mode too many chars in "Show Passwrd OFF", freezes device.
  * Crash when scrolling up the menu after selecting 'add account'.
  * failures are not showing correctly because you need to blank out the line 
    first
  * after adding an account we can't find it
  
  TODO / Enhancements
  ===================
    - = unimplemented
    ? - tried to implement, ran into problems
    % - concerned there isn't enough memory left to implement
    x = implemented but not tested  
    * - implemented and tested
  - consider halfing the number of possible accounts (from 256 to 128) and 
    doubling the size of the accountname, username, and password fields (from 32
    to 64).
  - learn how to set the lock bits
  - ground unused pins
  - Add the ability to fix a corrupt linked list.
  - salt the encrypted account name, username and password, probably with 4
    bytes associated with the position of the account.  This will reduce the 
    size of the master password from 15 to 11(?) or 12?
  ? add a feature whereby the unit factory resets after two triple clicks, even
    if not yet authenticated. (commented out, caused problems)
  ? add a feature whereby the unit logsout after two double clicks. (commented
    out, caused problems)
  % add a decoy password that executes a factory reset when the password plus
    the characters "FR" are supplied as the master password.
  % add the ability to pump a single tab or a single carrige return from the 
    menu.
  % make it possible to import creds via XML file
  % add the ability to change the master password
  x add salt to the hashed master password.
  x ensure we don't read more bytes than that which we can accomodate in the 
    buffer
  x encrypt the usernames (need to confirm by examining EEprom)
  x encrpt the account names (need to confirm by examining EEprom)
  * implement with a better font
  * work on the workflow; which menu items are defaulted after which events.
  * Delete account (account name, username, password)
  * substitute memcpy with for loops
  * implement a doubly linked list to keep the accounts in order
  * store the master password in internal EEprom and everything else in external
    EEprom.
  * reconsider the organization of the menus
  * ask for confirmation before performing destructive tasks
  * Restore backup (restore from the EEprom backup)
  * implement backup from external EEprom to external EEprom
  * incorporate the use of external EEprom chip to expand available memory
  * make it possible to edit creds via keyboard
  * Encrypt all passwords (except the master password)
  * finish the video
    https://screencast-o-matic.com/
  * decide if the encrypted pw will be saved to EEprom
  * get ScreenCastOMatic to record the video for the project 
  * add function(s) to send error output to the display device (e.g. no external 
    EEprom.
  * Make it possible to send the account name, where a URL can be entered.
  * eliminate the need for the acctsArray to conserve memory.
  * make room for a next pointer and a previous pointer to maintain a doubly 
    linked list so that we can sort the accountnames.
  * decide if you should display the passwords.  Possibly make it configurable.
  * mask passwords on input
  * add a feature that dumps all account names, usernames and passwords out
    through the keyboard (like to be inserted into an editor as a backup).
  * have the unit factory reset after 10 failed attempts.  Store the failed
    attempts in EEprom.
  * Hash the master password
  * A master key generated from the user password is hashed using SHA-256, which 
    is subsequently used to encrypt the password database with AES-128. 
    Cryptography experts no longer recommend CBC for use in newer designs. It 
    was an important mode in the past but newer designs should be using 
    authenticated encryption with associated data (AEAD) instead.   
  * have the unit automatically logout after a period of inactivity, this will
    require use of the timer.

  Warnings
  ========
  - Program memory is nearly full so be careful and watch out for flaky 
    behavior.  Once over 80% of global space you're in trouble. The LED library 
    is large.

  Suggestions
  ===========
  - Best viewed in an editor w/ 160 columns, most comments are at column 80
  - Please submit defects you find so I can improve the quality of the program
    and learn more about embedded programming.
  - For anyone unfamiliar w/ Arduino when the device is powered on first setup() 
    runs and then loop() runs, in a loop, in perpetuity.
  - Set tab spacing to 4 characters.

  Contributors
  ============
  smching: https://gist.github.com/smching/05261f11da11e0a5dc834f944afd5961 
  for EEPROMUtil.cpp.
  Source code has been pulled from all over the internet, it would be impossible 
  for me to cite all contributors.  Special thanks to Elliott Williams for his 
  essential book "Make: AVR Programming", which is highly recommended. 

  Copyright
  =========
  - Copyright ©2018, ©2019 Daniel J Murphy <dan-murphy@comcast.net>
  
  License
  =======
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Libraries 
  =========
  - https://rweather.github.io/arduinolibs/index.html - AES and SHA library
  - https://github.com/LennartHennigs/Button2 - Used for the button on the 
    rotary encoder
  - https://github.com/brianlow/Rotary - Used for the rotary encoder
  - https://github.com/arduino-libraries/Keyboard - Used to send characters to 
    the keyboard as if typed by the user
  - https://www.arduino.cc/en/Reference/EEPROM - Used for internal EEprom
  - https://github.com/greiman/SSD1306Ascii/blob/master/examples/AvrI2c128x32/AvrI2c128x32.ino
    for SSD1306 display device

  Components
  ==========
    Component                       Retail
    Description	                    Cost
    ===========                     =====
  - Arduino Pro Micro (ATMega32u4)  $2.8700         https://www.aliexpress.com/item/New-Pro-Micro-for-arduino-ATmega32U4-5V-16MHz-Module-with-2-row-pin-header-For-Leonardo/32768308647.html?spm=a2g0s.9042311.0.0.12914c4d0Xj2PY
  - RGB LED                          0.0200         https://www.aliexpress.com/item/Free-shipping-100pcs-5mm-RGB-LED-Common-Cathode-Tri-Color-Emitting-Diodes-f5-RGB-Diffused/32330160607.html?spm=2114.search0104.3.17.6ae840aaohPCBN&ws_ab_test=searchweb0_0,searchweb201602_3_10065_10068_10130_318_10890_10547_319_10546_10548_317_10545_10696_450_10084_10083_10618_452_535_534_533_10307_532_204_10059_10884_323_325_10887_100031_320_321_322_10103_448_449_5728415,searchweb201603_55,ppcSwitch_0&algo_expid=106caae9-74c0-4f4a-b224-e96dd4f8efb1-2&algo_pvid=106caae9-74c0-4f4a-b224-e96dd4f8efb1&transAbTest=ae803_5
  - Momentary push button            0.0084         https://www.aliexpress.com/item/100pcs-lot-Mini-Micro-Momentary-Tactile-Push-Button-Switch-6-6-5mm-4-pin-ON-OFF/32858344336.html?spm=2114.search0104.3.25.2c612f04SiR6k5&ws_ab_test=searchweb0_0,searchweb201602_3_10065_10130_10068_10890_10547_319_10546_317_10548_10545_10696_453_10084_454_10083_10618_10307_537_536_10902_10059_10884_10887_321_322_10103,searchweb201603_55,ppcSwitch_0&algo_expid=6e5b9235-2049-47a5-b3d4-024991e840f0-3&algo_pvid=6e5b9235-2049-47a5-b3d4-024991e840f0&transAbTest=ae803_5
  - Custom PCB                       1.1000         https://www.aliexpress.com/item/2PCS-PCB-Board-Stripboard-Veroboard-Prototype-Card-Prototyping-Circuit-Double-Sided-Universal-DOT-Perfboard-Breadboard-Bricolage/32880212821.html?spm=2114.search0104.3.10.5d17703cU0we8a&ws_ab_test=searchweb0_0,searchweb201602_3_10065_10068_10130_318_10890_10547_319_10546_10548_317_10545_10696_450_10084_10083_10618_452_535_534_533_10307_532_204_10059_10884_323_325_10887_100031_320_321_322_10103_448_449_5728415-10890,searchweb201603_55,ppcSwitch_0&algo_expid=938e72f3-c597-4efc-b3be-66aeeef7dc47-1&algo_pvid=938e72f3-c597-4efc-b3be-66aeeef7dc47&transAbTest=ae803_5
  - Rotary Encoder                   0.4200         https://www.aliexpress.com/item/5pcs-Rotary-encoder-code-switch-EC11-audio-digital-potentiometer-with-switch-5Pin-handle-length-15mm/32798669185.html?spm=2114.search0104.3.1.4bc224f8PBVffZ&ws_ab_test=searchweb0_0,searchweb201602_3_10065_10068_10130_318_10890_10547_319_10546_10548_317_10545_10696_450_10084_10083_10618_452_535_534_533_10307_532_204_10059_10884_323_325_10887_100031_320_321_322_10103_448_449_5728415,searchweb201603_55,ppcSwitch_0&algo_expid=601795c0-e796-4076-a084-03df4f6bab0f-0&algo_pvid=601795c0-e796-4076-a084-03df4f6bab0f&transAbTest=ae803_5
  - Knob                             0.1500         https://www.aliexpress.com/item/10pcs-Plastic-Volume-Control-Knob-Potentiometer-Knob-Cap-for-Encoder-Potentiometer-6mm-Round-Shaft-WH148/32811813896.html?spm=a2g0s.9042311.0.0.15554c4d2B1pdZ
  - I2C LED display 32x128 pixels.   1.6400         https://www.aliexpress.com/item/ShengYang-1pcs-0-91-inch-OLED-module-0-91-white-blue-OLED-128X32-OLED-LCD-LED/32927682460.html?spm=2114.search0104.3.8.60765daeMuRsBD&ws_ab_test=searchweb0_0,searchweb201602_3_5731312_10065_10068_10130_10890_10547_319_10546_317_10548_5730312_10545_10696_5728812_10084_10083_5729212_10618_5731412_10307_5731212_5731112_328_10059_10884_5732012_5731512_10887_100031_5731612_321_322_10103_5732512_5731712,searchweb201603_55,ppcSwitch_0&algo_expid=f653859a-2805-475d-85a3-b236a7dae281-1&algo_pvid=f653859a-2805-475d-85a3-b236a7dae281
  - 2 25LC256 External EEprom        0.4100         https://trade.aliexpress.com/orderList.htm?spm=2114.11010108.1000002.15.650c649beSGpy2&tracelog=ws_topbar
  - 3 220ohm resistors               0.0420         https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20181024040631&SearchText=resistors	
  - 2 4.7kohm resistors              0.0280         https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20181024040631&SearchText=resistors	
  - Case                             ?.????
  - Assembly                         ?.????
    -----
  - Total                           $6.69

  
  Hardware
  ========
  - 1 SparkFun Pro Micro (w/ ATMega32u4 microcontroller)
  - Data Sheet: https://www.pjrc.com/teensy/atmega32u4.pdf
    Number Name                 Connect To / Notes
    1       TX    D1    PD3     
    2       RX    D0    PD2     
    3       GND                 
    4       GND                 
    5       SDA   D2    PD1     SSD1306 SDA, 4.7k pullup
    6       SCL   D3    PD0     SSD1306 SCL, 4.7k pullup
    7       A6    D4    PD4     
    8             D5    PC6     
    9       A7    D6    PD7     pin 1 backup chip select for 25LC256
    10            D7    PE6     rotary button
    11      A8    D8    PB4     rotary pin 2
    12      A9    D9    PB5     rotary pin 1
    13      A10   D10   PB6     pin 1 primary chip select for 25LC256
    14      MOSI  D16   PB2     pin 5 primary 25LC256, backup 25LC256
    15      MISO  D14   PB3     pin 2 primary 25LC256, backup 25LC256
    16      SCLK  D15   PB1     pin 6 primary 25LC256, backup 25LC256
    17      A0    D18   PF7     must float    Used for random # generator
    18      A1    D19   PF6     red RGB pin
    19      A2    D20   PF5     green RGB pin
    20      A3    D21   PF4     blue RGB pin
    21      Vcc (+3V)           TODO: should 25LC256 VCC connect to pin 24 
                                instead (5 volts)?, SSD1306 Vcc?
    22      Reset               reset button
    23      GND                 GND RGB, GND SSD1306, GND rotary encoder button, 
                                GND 2 25LC256 chips
    24      RAW (+5V from USB)  should we be pulling power for the components 
                                from here?

  - 2 25LC256 (External EEprom)
    Tested Part: MICROCHIP - 25LC256-I/P - 256K SPI™ Bus Serial EEPROM DIP8
  - Data Sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20005715A.pdf

    Number Name                 ConnectTo        Note
    1       CS    D10 PB6       pin 13 promicro  Chip Select Input
    2       SO    D14           pin 15 promicro  MISO - Serial Data Output
    3       WP    VCC           pin 21 promicro  Write Protect
    4       Vss   GND           pin 23 promicro  Ground
    5       SI    D16           pin 14 promicro  MOSI - Serial Data Input
    6       SCK   D15           pin 16 promicro  SCLK - Serial Clock Input
    7       HOLD  VCC           pin 21 promicro  Hold Input
    8       Vcc   VCC           pin 21 promicro  Supply Voltage 

    Number Name                 ConnectTo        Note
    1       CS    D10 PD7       pin 06 promicro  Chip Select Input
    2       SO    D14           pin 15 promicro  MISO - Serial Data Output
    3       WP    VCC           pin 21 promicro  Write Protect
    4       Vss   GND           pin 23 promicro  Ground
    5       SI    D16           pin 14 promicro  MOSI - Serial Data Input
    6       SCK   D15           pin 16 promicro  SCLK - Serial Clock Input
    7       HOLD  VCC           pin 21 promicro  Hold Input
    8       Vcc   VCC           pin 21 promicro  Supply Voltage 

  RGB Colors and Meanings
  =======================
  Green                 Logged in
  Blue                  Not logged in 
  Red                   Failed login attempt
                        Error backing up or initializing EEprom
  Orange                Backing up EEprom memory, initializing EEprom
  Purple                Sending creds
  Yellow                Backing up to EEprom
  Fast Flash Red / Blue Initializing external EEprom
  Slow flash Red / Blue Initializing internal EEprom
  
  Budgeting Memory
  ================
  - Current Setup 
  ---------------
  Sketch uses 27756 bytes (96%) of program storage space. Maximum is 28672 
  bytes. Global variables use 1685 bytes (65%) of dynamic memory, leaving 875 
  bytes for local variables. Maximum is 2560 bytes.

  Menu Navigation
  ===============
  Master Password                  STATE_ENTER_MASTER   ->STATE_SHOW_MAIN_MENU
  Find Account                     STATE_SHOW_MAIN_MENU ->STATE_FIND_ACCOUNT
    [scroll through accounts list] STATE_FIND_ACCOUNT   ->STATE_SEND_CREDS_MENU
      Send User & Pass             STATE_SEND_CREDS_MENU
      Send Password                STATE_SEND_CREDS_MENU
      Send Username                STATE_SEND_CREDS_MENU
      Send Account                 STATE_SEND_CREDS_MENU
      Edit Credentials             STATE_SEND_CREDS_MENU
        Account Name               STATE_EDIT_CREDS_MENU->STATE_EDIT_ACCOUNT
        Edit Username              STATE_EDIT_CREDS_MENU->STATE_EDIT_USERNAME
        Edit Password              STATE_EDIT_CREDS_MENU->STATE_EDIT_PASSWORD
        Indicate Style             STATE_EDIT_CREDS_MENU->STATE_EDIT_STYLE
        GeneratePasswrd            STATE_EDIT_CREDS_MENU->
      Delete Account [confirm]     STATE_SEND_CREDS_MENU->STATE_CONFIRM_DEL_ACCT
  Add Account                      STATE_SHOW_MAIN_MENU
    Account Name                   STATE_EDIT_CREDS_MENU
    Edit Username                  STATE_EDIT_CREDS_MENU
    Edit Password                  STATE_EDIT_CREDS_MENU
    Indicate Style                 STATE_EDIT_CREDS_MENU
    GeneratePasswrd                STATE_EDIT_CREDS_MENU
  Logout                           STATE_SHOW_MAIN_MENU
  Keyboard ON/OFF                  STATE_SHOW_MAIN_MENU
  Show Passwrd ON/OFF              STATE_SHOW_MAIN_MENU
  Backup EEprom [confirm]          STATE_SHOW_MAIN_MENU->STATE_CONFIRM_BACK_EEPROM
  Backup to File                   STATE_SHOW_MAIN_MENU
  Restore Backup [confirm]         STATE_SHOW_MAIN_MENU->STATE_CONFIRM_RESTORE
  Fix Corruption [confirm]         STATE_SHOW_MAIN_MENU->STATE_CONFIRM_FIX_CORRUPT
  Reset [confirm]                  STATE_SHOW_MAIN_MENU->STATE_CONFIRM_RESET
  
  The Program 
  ===========
  - Includes/Defines                                                            */
#define F_CPU                     16000000UL                                    // microcontroller clockspeed

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "Button2.h";                                                           // for the button on the rotary encoder
#include <Rotary.h>                                                             // for the rotary encoder
#include <Keyboard.h>                                                           // for simulating a USB keyboard and sending output to it
#include <EEPROM.h>                                                             // for reading and writing AtMega32u4 internal EEprom
#include <SHA256.h>                                                             // for hashing the master password
#include <AES.h>                                                                // for encrypting credentials
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

//- Macros

#define EnableInterrupts()        sei()
#define DisableInterrupts()       cli()
#define getLoginFailures          read_eeprom_byte(GET_ADDR_LOGIN_FAILURES)
#define getResetFlag              read_eeprom_byte(GET_ADDR_RESET_FLAG)
#define getShowPasswordsFlag      read_eeprom_byte(GET_ADDR_SHOW_PW)
//#define getKeyboardFlag           read_eeprom_byte(GET_ADDR_KEYBOARD_FLAG)
//#define writeLoginFailures        write_eeprom_byte(GET_ADDR_LOGIN_FAILURES, loginFailures)

//- Pins

#define ROTARY_PIN1               9                                             // for AtMega32u4 / Arduino Pro Mini
#define ROTARY_PIN2               8                                             //   "                               
#define BUTTON_PIN                7                                             //   "                              
#define RED_PIN                   21                                            // Pin locations for the RGB LED, must be PWM capable (was 19)
#define BLUE_PIN                  19                                            // R - 21, B - 19, G 20 for Rev. 7 of PCB (was 21)
#define GREEN_PIN                 20                                            // (was 20)
#define ADC_READ_PIN              18                                            // we read the voltage from this floating pin to seed the random number generator, don't ground it!

//- BAUD Rate

#define BAUD_RATE                 38400                                         // Baud rate for the Serial monitor, best for 16MHz

//- Entering data via Rotary Encoder
                                                                               
#define NO_LED_LIB_CHAR_WIDTH_PIX 6                                             // when entering chars via rotary encoder, the amount to advance the cursor for each character already entered
#define FIXED_CHAR_SPACING        10                                            // when entering chars via rotary encoder, the fixed amount to advance the cursor

#define SHA_ITERATIONS            1                                             // number of times to hash the master password (won't work w/ more than 1 iteration)

//- Special Byte Values

#define INITIAL_MEMORY_STATE_CHAR -1                                            // 11111111 binary twos complement, -1 decimal, 0xFF hex.  When factory fresh all bytes in EEprom memory = 0xFF.
#define INITIAL_MEMORY_STATE_BYTE 0xFF                                          // 11111111 binary twos complement, -1 decimal, 0xFF hex.  When factory fresh all bytes in EEprom memory = 0xFF.
#define NULL_TERM                 0x00                                          // The null terminator, NUL, ASCII 0, or '\0'                 

//- SPI

#define SPI_SS_PRIMARY            PB6                                           // chip select primary (copy source)  (SPI)
#define SPI_SS_PRIMARY_PORT       PORTB
#define SPI_SS_PRIMARY_PIN        PINB
#define SPI_SS_PRIMARY_DDR        DDRB
#define SPI_SS_SECONDARY          PD7                                           // chip select seconday (copy target)  (SPI)
#define SPI_SS_SECONDARY_PORT     PORTD                                         // 
#define SPI_SS_SECONDARY_PIN      PIND                                          // 
#define SPI_SS_SECONDARY_DDR      DDRD                                          // 
#define SPI_MOSI                  PB2                                           // mosi (SPI)
#define SPI_MOSI_PORT             PORTB
#define SPI_MOSI_PIN              PINB
#define SPI_MOSI_DDR              DDRB
#define SPI_MISO                  PB3                                           // miso (SPI)
#define SPI_MISO_PORT             PORTB
#define SPI_MISO_PIN              PINB
#define SPI_MISO_DDR              DDRB
#define SPI_SCK                   PB1                                           // clock (SPI)
#define SPI_SCK_PORT              PORTB
#define SPI_SCK_PIN               PINB
#define SPI_SCK_DDR               DDRB

#define SLAVE_PRIMARY_SELECT      SPI_SS_PRIMARY_PORT &= ~(1<<SPI_SS_PRIMARY);
#define SLAVE_PRIMARY_DESELECT    SPI_SS_PRIMARY_PORT |= (1<<SPI_SS_PRIMARY)
#define SLAVE_SECONDARY_SELECT    SPI_SS_SECONDARY_PORT &= ~(1<<SPI_SS_SECONDARY);
#define SLAVE_SECONDARY_DESELECT  SPI_SS_SECONDARY_PORT |= (1<<SPI_SS_SECONDARY)

// Instruction Set -- from data sheet

#define EEPROM_READ               0b00000011                                    // read memory
#define EEPROM_WRITE              0b00000010                                    // write to memory
#define EEPROM_WREN               0b00000110                                    // write enable
#define EEPROM_RDSR               0b00000101                                    // read status register

                                                                                // EEPROM Status Register Bits -- from data sheet
                                                                                // Use these to parse status register
#define EEPROM_WRITE_IN_PROGRESS  0
#define EEPROM_WRITE_ENABLE_LATCH 1
#define EEPROM_BLOCK_PROTECT_0    2
#define EEPROM_BLOCK_PROTECT_1    3

//- Memory Layout

#define MEMORY_INITIALIZED_FLAG   0x01                                          // signals if memory has been initialized correctly
#define EEPROM_BYTES_PER_PAGE     0x20                                          // 32. can't exceed 255 (real page size is 64 for 25LC256)
#define DISPLAY_BUFFER_SIZE       EEPROM_BYTES_PER_PAGE                         // 32
#define MAX_AVAIL_ADDR            0x7FFF                                        // 32,767. 25LC256 = 256kbits capacity.
#define MIN_AVAIL_ADDR            0x00                                          // assuming we start at the very beginning of EEprom
#define ACCOUNT_SIZE              EEPROM_BYTES_PER_PAGE                         // bytes, put on the 1/2 page boundry
#define USERNAME_SIZE             EEPROM_BYTES_PER_PAGE                         // bytes, put on the 1/2 page boundry
#define CRED_SALT_SIZE            0x02                                          // 2 bytes, a uint16_t.  size of key for aes128 == 16 bytes.  2 bytes will be for salt. range= 0 - 65,535
#define PASSWORD_SIZE             EEPROM_BYTES_PER_PAGE                         // 32 bytes
#define STYLE_SIZE                0x02                                          // bytes, we are storing the null terminator
#define PREV_POS_SIZE             0x01                                          // bytes, datatype byte, no null terminator
#define NEXT_POS_SIZE             0x01                                          // bytes, datatype byte, no null terminator
#define CREDS_TOT_SIZE            0x80                                          // 128.  leaving an extra 26 bytes on the end so we're on the page boundry
#define MASTER_PASSWORD_SIZE      (0x10 - CRED_SALT_SIZE)                       // aes256 keysize = 32 bytes.  aes128 keysize = 16 bytes, aes256 blocksize = 16!, only the first 15 chars are part of the password, the rest are ignored.
#define HASHED_MASTER_PASSWORD_SZ (MASTER_PASSWORD_SIZE * 2)                    // the size of the hashed master password
#define LOGIN_FAILURES_SIZE       1
#define SHOW_PASSWORD_FLAG_SIZE   1
#define LIST_HEAD_SIZE            1
#define GET_ADDR_RESET_FLAG       MAX_AVAIL_ADDR                                // address of the reset flag; when not set to 0x01 indicates that memory hasn't been initialized; 32,768
#define CREDS_ACCOMIDATED         (INITIAL_MEMORY_STATE_BYTE - 1)               // 254 is max for the 25LC256 with the configuration related values stored at the end. Can't exceed 255. TODO: calculate (MAX_AVAIL_ADDR + 1) / CREDS_TOT_SIZE.  Use 254 because 255 == INITIAL_MEMORY_STATE_BYTE.
#define GET_ADDR_ACCT(pos)        (MIN_AVAIL_ADDR + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_USER(pos)        (MIN_AVAIL_ADDR + ACCOUNT_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_PASS(pos)        (MIN_AVAIL_ADDR + ACCOUNT_SIZE + USERNAME_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_STYLE(pos)       (MIN_AVAIL_ADDR + ACCOUNT_SIZE + USERNAME_SIZE + PASSWORD_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_PREV_POS(pos)    (MIN_AVAIL_ADDR + ACCOUNT_SIZE + USERNAME_SIZE + PASSWORD_SIZE + STYLE_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_NEXT_POS(pos)    (MIN_AVAIL_ADDR + ACCOUNT_SIZE + USERNAME_SIZE + PASSWORD_SIZE + STYLE_SIZE + PREV_POS_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_CRED_SALT(pos)   (MIN_AVAIL_ADDR + ACCOUNT_SIZE + USERNAME_SIZE + PASSWORD_SIZE + STYLE_SIZE + PREV_POS_SIZE + NEXT_POS_SIZE + (pos * CREDS_TOT_SIZE))
#define GET_ADDR_SETTINGS         (MAX_AVAIL_ADDR - ((EEPROM_BYTES_PER_PAGE * 2) - 1))// use the last page for storing the settings.  TODO: move this to internal EEprom so it is protected by lock bits. 32,704 (cannot be < 32,640)
#define GET_ADDR_LOGIN_FAILURES   (GET_ADDR_SETTINGS)                           //
#define GET_ADDR_SHOW_PW          (GET_ADDR_SETTINGS + LOGIN_FAILURES_SIZE)     //
#define GET_ADDR_LIST_HEAD        (GET_ADDR_SETTINGS + LOGIN_FAILURES_SIZE + SHOW_PASSWORD_FLAG_SIZE ) // points to the head of the linked list
#define MAX_AVAIL_INT_ADDR        0x03FF                                        // 1,023 is the max address of the EEprom on AtMega32u4
#define MIN_AVAIL_INT_ADDR        0x00                                          // assuming we start at the very beginning of EEprom
#define GET_ADDR_MASTER_HASH      (MAX_AVAIL_INT_ADDR - HASHED_MASTER_PASSWORD_SZ)// store hashed master password near the end of EEprom (sneaky)
#define GET_ADDR_SALT             (GET_ADDR_MASTER_HASH - MASTER_PASSWORD_SIZE) // location of the salt for the SHA hash

//- Events
                                                                                // Assumption here is that using #define instead of enum will save memory
#define EVENT_NONE                0                                             // used to switch of the previous event to avoid infinate looping
#define EVENT_SINGLE_CLICK        1                                             // a single click on the rotary encoder
#define EVENT_LONG_CLICK          4                                             // holding the button on the rotary encoder down for more than 1 second(?) (changed from default directly in the library)
#define EVENT_ROTATE_CW           7                                             // turning the rotary encoder clockwise
#define EVENT_ROTATE_CC           8                                             // turning the rotary encoder counter clockwise.
#define EVENT_SHOW_MAIN_MENU      9                                             // to show the main menu
#define EVENT_SHOW_EDIT_MENU      11                                            // to show the menu used for editing account name, username and password (creds)
#define EVENT_RESET               12                                            // Factory Reset event
#define EVENT_LOGOUT              13                                            // logging out of the device
#define EVENT_BACKUP              14                                            // copying the content of the primary external EEprom to the backup EEprom
#define EVENT_RESTORE             15                                            // restore from the backup EEprom to the primary EEprom
#define EVENT_BACKUP_TO_FILE      16                                            // send all creds out through the keyboard for capture in a text editor
//#define EVENT_FIX_CORRUPTION    17                                            // fix a corrupt linked list
#define EVENT_DELETE_ACCT         18                                            // delete an account
                                                                                // Not using an enum here to save memory.  TODO: states are not really mutually
//- States                                                                      // exclusive so we could save a byte by just numbering these sequentially.

#define STATE_ENTER_MASTER        1                                             // entering the master password
#define STATE_SHOW_MAIN_MENU      2                                             // showing the main menu                              
#define STATE_FIND_ACCOUNT        3                                             // searching for an account                           
#define STATE_EDIT_STYLE          4                                             // editing the style for sending username and password
#define STATE_EDIT_USERNAME       5                                             // entering the username                              
#define STATE_EDIT_PASSWORD       6                                             // entering the password                              
#define STATE_EDIT_CREDS_MENU     7                                             // showing the edit menu                              
#define STATE_EDIT_ACCOUNT        8                                             // entering the account name                          
#define STATE_SEND_CREDS_MENU     9                                             // showing the menu that sends creds via keyboard     
#define STATE_CONFIRM_BACK_EEPROM 10                                            // confirming an action                               
#define STATE_CONFIRM_RESTORE     11                                            // confirming restore from backup EEprom              
//#define STATE_CONFIRM_FIX_CORRUPT 12                                          // confirming fix corruption function                 
#define STATE_CONFIRM_DEL_ACCT    13                                            // confirming account/credentials delete              
#define STATE_CONFIRM_RESET       14                                            // confirming factory reset                           

//- I2C Address

#define SSD1306_I2C_ADDR          0x3C                                          // Slave 132x64 OLED I2C address
                                                                                
//- Time

#define MAX_IDLE_TIME             3600000                                       // one hour; the idle time allowed before automatic logout
#define LONG_CLICK_LENGTH         500                                           // milliseconds to hold down the rotary encoder button to trigger EVENT_LONG_CLICK
#define UN_PW_DELAY               3000                                          // time in milliseconds to wait after entering username before entering password
#define SHOW_SPLASHSCREEN         3000                                          // in microseconds
//#define SHOW_COPYRIGHT          1000

//- Menus (globals)

#define KBD_MENU_O_POS            10                                            // where to append "FF" or "N" on the "Keyboard O" menu item
#define SH_PW_MENU_O_POS          13                                            // where to append "FF" or "N" on the  "Show Psswrd O" menu item
#define MENU_SIZE                 10                                            // selections in the menu
#define MAIN_MENU_NUMBER          0
#define MAIN_MENU_ELEMENTS        10                                            // number of selections in the main menu
                                                                            
char  * mainMenu[] =      {                      "Master Password",             // menu picks appear only on the top line
                                                 "Find Account",                // after an account is found send user sendMenu menu
                                                 "Add Account",
                                                 "Logout",                      // locks the user out until master password is re-entered
                                                 "Keyboard O  ",                // flag that determines if the input by keyboard feature is on or off
                                                 "Show Psswrd O  ",             // determines if passwords are displayed or not
                                                 "Backup EEprom",               // duplicate the external EEprom
                                                 "Backup to File",              // sends all credential out through the keyboard for capture in an editor
                                                 "Restore Backup",              // copies the content of the secondary EEprom back to the primary, overwriting.
//                                               "Fix Corruption",              // fix any corruption in the linked list
                                                 "Reset"            };          // factory reset; erases all creds from memory

#define ENTER_MASTER_PASSWORD     0                                             // locations of the main menu items
#define FIND_ACCOUNT              1
#define ADD_ACCOUNT               2
#define LOGOUT                    3
#define SET_KEYBOARD              4
#define SET_SHOW_PASSWORD         5
#define BACKUP_EEPROM             6
#define BACKUP_ALL                7
#define RESTORE_BACKUP            8
//#define FIX_CORRUPT_LIST        9
#define FACTORY_RESET             9

uint8_t menuNumber = MAIN_MENU_NUMBER;                                          // holds the menu number of the currently displayed menu
uint8_t elements = MAIN_MENU_ELEMENTS;                                          // holds the number of selections in the currently displayed menu
char *currentMenu[MENU_SIZE];                                                   // holds the content of the currently displayed menu

#define SEND_MENU_NUMBER          1
#define SEND_MENU_ELEMENTS        7                                             // number of selections in the send creds menu
const char * const sendMenu[] =       {          "Send User & Pass",            // menu picks appear only on the top line
                                                 "Send Password <RET>",         // sends the password then a carriage return
                                                 "Send Username",
                                                 "Send Password",               // sends just the password w/ no carriage return (mostly for changing password)
                                                 "Send Acct",
                                                 "Edit Creds",                  // sends user to enterMenu menu
                                                 "Delete Acct",                 // delete the account
                                                 ""                 };

#define SEND_USER_AND_PASSWORD    0                                             // locations of the send credentials menu items
#define SEND_PASSWORD             1
#define SEND_USERNAME             2
#define SEND_PASSWORD_NO_RET      3
#define SEND_ACCOUNT              4
#define EDIT_ACCOUNT              5
#define DELETE_ACCOUNT            6

#define EDIT_MENU_NUMBER          2
#define EDIT_MENU_ELEMENTS        5                                             // the number of selections in the menu for editing credentials
const char * const enterMenu[] =       {         "Account Name",                // menu picks appear only on the top line
                                                 "Edit Username",  
                                                 "Edit Password",
                                                 "Indicate Style",              // 0, <CR>, 1, <TAB> between username and password when both sent
                                                 "Gen Password",
                                                 ""                 };

#define EDIT_ACCT_NAME            0                                             // locations of the edit credentials menu items
#define EDIT_USERNAME             1
#define EDIT_PASSWORD             2
#define EDIT_STYLE                3
#define GENERATE_PASSWORD         4

//- Global Variables                                                            // char is signed by default. byte is unsigned.

uint8_t accountName[ACCOUNT_SIZE];                                              // holds the account name of the current account
uint8_t username[USERNAME_SIZE];                                                // holds the username of the current account
uint8_t password[PASSWORD_SIZE];                                                // holds the password of the current account
uint8_t credSalt[CRED_SALT_SIZE];                                               // the salt for the current account
uint8_t style[STYLE_SIZE];                                                      // holds the style of the current account (<TAB> or <CR> between send username 
                                                                                // and password)

#define LEN_ALL_CHARS             94
#define DEFAULT_ALPHA_EDIT_POS    40                                            // allChars is sort of unnecessary TODO: eliminate allChars?
#define DEFAULT_STYLE_EDIT_POS    30
const char allChars[LEN_ALL_CHARS] =                                            // used to edit text via rotary encoder (164 bytes)
" /?><,:';|}{][+_)(*&^%$#!~=\-@. 0123456789AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz"; 
const char confirmChars[2] = "NY";                                              // used to select N(o) or Y(es) via rotary encoder to confirm a destructive action
#define POS_Y_N_CONFIRM           14                                            // x pixel position of Y and N for the confirm menu item

char line1DispBuff[DISPLAY_BUFFER_SIZE];                                        // used to buffer output of line 1 for the led display
char line2DispBuff[DISPLAY_BUFFER_SIZE];                                        // used to buffer output of line 2 for the led display

const char spaceFilled16[] = "               ";                                 // 15 chars long to leave room for the null terminator

#define TAB_KEY                   KEY_TAB                                       // TAB key is ascii 0x2b (not 0x09) or 0x2b, 0xB3; KEY_TAB from Keyboard.h, 0xB3

uint8_t masterPassword[MASTER_PASSWORD_SIZE];                                   // this is where we store the master password for the device

uint8_t loginFailures;                                                          // count of the number of consecutive login failures since the last successful 
                                                                                // password entry.
#define MAX_LOGIN_FAILURES        10                                            // "Factory Reset" after MAX_LOGIN_FAILURES attempts to login. Gurads against 
                                                                                // brute force attack.
uint8_t showPasswordsFlag;                                                      // flag indicating if we show passwords via the UI, or hide them.
uint8_t keyboardFlag;                                                           // flag indicating if we're using the keyboard to edit creds
uint8_t addFlag = false;                                                        // tracks wheter we reached the Add Account menu via the main menu or the find menu,
                                                                                // which is necessary to determine where to return to on EVENT_LONG_CLICK.
//- Global Volatile variables.

volatile uint8_t event = EVENT_NONE;                                            // this is the only variable manipulated in the interrupt.

//- Globals associated with state

uint8_t machineState;                                                           // the state of the device
                                                                                // it is presently unnecessarily long.  Can all be stored in uint8_t otherwise.
int position = 0;                                                               // the position of the rotary encoder, used to navigate menus and enter text.
uint8_t enterPosition = 0;                                                      // when alpha editing w/ rotary encoder, position in the edited word
uint8_t acctPosition;                                                           // the positon of the selected account.
uint8_t acctCount = 0;                                                          // the number of accounts in EEprom.
boolean authenticated = false;                                                  // indicates if the correct master password has been provided
unsigned long lastActivityTime;                                                 // used to automatically logout after a period of inactivity
uint8_t iterationCount = 0;                                                     // # of times ProcessEvent() called since last evaluation of lastActivityTime
uint8_t headPosition;                                                           // the head of the doubly linked list that keeps account names sorted
uint8_t tailPosition;                                                           // the tail of the doubly linked list that keeps account names sorted

//- Object setup

SHA256 sha256;
AESSmall128 aes;                                                                // 16 byte key, 32 byte block
//AESSmall256 aes;                                                              // 32 byte key, 32 byte block; this uses 4% more program memory. Set 
                                                                                // MASTER_PASSWORD_SIZE = 32 when in use.
Rotary rotaryEncoder = Rotary(ROTARY_PIN1, ROTARY_PIN2);                        // the rotary encoder object.
Button2 encoderButton = Button2(BUTTON_PIN);                                    // the button on the rotary encoder.
SSD1306AsciiAvrI2c oled;

//- Function Prototypes

void setup(void);                                                               // first function run by device
void loop(void);                                                                // iterates forever
void ProcessEvent(void) ;                                                       // to handle events as they arise
void ConfirmChoice(int state);                                                  // confirms a Y/N choice input via rotary encoder
void ReadFromSerial(char *buffer, uint8_t size, char *prompt);
void switchToEditMenu(void);
void switchToSendCredsMenu(void);
void switchToFindAcctMenu(void);
void SwitchRotatePosition(uint8_t pos);
void FactoryReset(void);
//void ISR(uint8_t PCINT0_vect);                                                // error: expected unqualified-id before string constant
void buttonReleasedHandler(Button2& btn);
void deleteAccount(uint8_t position);
void setUUID(uint8_t *password, uint8_t size, uint8_t appendNullTerm);
void sendAccount(void);
void sendUsername(void);
void sendPassword(void);
void sendRTN(void);
void sendUsernameAndPassword(void);
void sendAll(void);
void DisplayLine1(char* lineToPrint);
void DisplayLine2(char* lineToPrint);
void BlankLine1(void) ;
void BlankLine2(void);
void DisplayBuffer(void);
void ShowMenu(uint8_t position, char **menu);
void MenuUp(char **menu);
void MenuDown(char **menu);
void ShowChar(char charToShow, uint8_t  pos) ;
void flipOnOff(uint8_t flag, uint8_t pos, uint8_t startInx) ;
void setPurple(void);
void setRed(void);
void setGreen(void);
void setYellow(void);
void setBlue(void);
void setColor(uint8_t  redValue, uint8_t  greenValue, uint8_t  blueValue);
boolean authenticateMaster(uint8_t *password);
void sha256Hash(char *password);
void encrypt32Bytes(uint8_t *outBuffer, uint8_t *inBuffer);
void decrypt32(uint8_t *outBuffer, uint8_t *inBuffer);
void writeAllToEEProm(uint8_t *accountName, 
                      uint8_t *username, 
                      uint8_t *password, 
                      uint8_t pos) ;
void countAccounts(void) ;
uint8_t getNextFreeAcctPos(void) ;
void readAcctFromEEProm(uint8_t pos, uint8_t *buf);
void readUserFromEEProm(uint8_t pos, uint8_t *buf);
void readStyleFromEEProm(uint8_t pos, char *buf) ;
void readPassFromEEProm(uint8_t pos, uint8_t *buf);
void readCredSaltFromEEProm(uint8_t pos, uint8_t *buf);
uint8_t getListHeadPosition(void);
uint8_t getNextPtr(uint8_t pos);
uint8_t getPrevPtr(uint8_t pos);
void writeNextPtr(uint8_t pos, uint8_t nextPtr);
void writePrevPtr(uint8_t pos, uint8_t prevPtr);
void writeLoginFailures(void);
void writeResetFlag(uint8_t buf);
void writeShowPasswordsFlag(void);
//void writeKeyboardFlag(void);
void writeListHeadPos(void);
boolean eeprom_is_addr_ok(unsigned int addr);
boolean eeprom_write_bytes( unsigned int startAddr, 
                            const uint8_t* buf, 
                            uint8_t numBytes);
boolean eeprom_write_int_bytes( unsigned int startAddr,
                                const uint8_t* buf,
                                uint8_t numBytes);
void eeprom_read_int_string( unsigned int addr,
                             unsigned char* buffer, 
                             uint8_t bufSize);
void InitializeEEProm(void);
void InitializeIntEEProm(void);
void initSPI(void);                                                             // Init SPI to run EEPROM with phase, polarity = 0,0
void SPI_tradeByte(uint8_t byte);                                               // Generic.  Just loads up HW SPI register and waits
void EEPROM_send16BitAddress(uint16_t address);                                 // splits 16-bit address into 2 bytes, sends both
uint8_t EEPROM_readStatus(void);                                                // reads the EEPROM status register
uint8_t EEPROM_readStatusSecondary(void);
void EEPROM_writeEnable(void);                                                  // helper: sets EEPROM write enable
uint8_t read_eeprom_byte(uint16_t address);                                     // gets a byte from a given memory location
void read_eeprom_array( uint16_t address, 
                        uint8_t *buffer, 
                        uint8_t sizeOfBuffer,
                        uint8_t primaryFlag);
void write_eeprom_byte(uint16_t address, uint8_t byte) ;                        // writes a byte to a given memory location
void write_eeprom_array(uint16_t address, 
                        uint8_t *buffer, 
                        uint8_t sizeOfBuffer);
void CopyChip(uint8_t restoreFlag);
void writePointers(uint8_t accountPosition, uint8_t *accountName);
//void FixCorruptLinkedList(void);
void setCredSalt(uint8_t *credSalt, uint8_t size);
void ProcessAttributeInput( uint8_t *attributeName, 
                            uint8_t attributeSize, 
                            char    *menuName, 
                            uint8_t nextPosition,
                            uint8_t acctFlag,
                            uint16_t address          );
void enterAttributeChar(uint8_t *attribute, uint8_t passwordFlag);
void EditAttribute(uint8_t aState, uint8_t pos);
void setKey(uint8_t pos);

//- Main Program Control

void setup() {                                                                  // runs first when the device is powered on
//  Serial.begin(BAUD_RATE);                                                    // uncomment when debugging
//  while(!Serial);                                                             // uncomment when debugging
//  Serial.println("PP");                                                       // uncomment when debugging

  pinMode(RED_PIN,   OUTPUT);                                                   // RGB LED pins
  pinMode(GREEN_PIN, OUTPUT);                                                   // "
  pinMode(BLUE_PIN,  OUTPUT);                                                   // "
  pinMode(BUTTON_PIN, INPUT_PULLUP);                                            // setup button pin for input enable internal 20k pull-up resistor, goes LOW 
                                                                                // when pressed, HIGH when released
  pinMode(ADC_READ_PIN, INPUT);                                                 // this pin will float in a high impedance/Hi-Z state and it's voltage
                                                                                // will be read with every spin to seed the random number generator.
  randomSeed(analogRead(ADC_READ_PIN));                                         // do not ground this pin; use this or randomSeed(millis()); used for password 
                                                                                // generation
  encoderButton.setReleasedHandler(buttonReleasedHandler);                      // fires when button is released

  oled.begin(&Adafruit128x32, SSD1306_I2C_ADDR);
  oled.setFont(TimesNewRoman13);                                                // perfect, slightly smaller than Arial14
//oled.setFont(Arial14);                                                        // perfect but it's trueType so could be an issue
//oled.setFont(fixednums8x16);                                                  // Doesn't work, blank screen
//oled.setFont(Arial_bold_14);                                                  // Nice but a bit too big
//oled.setFont(Adafruit5x7);                                                    // Nice but a bit small.  Can get 3 lines of output.
//oled.setFont(Adafruit5x7);                                                    // Nice but a bit small.  Can get 3 lines of output.
  DisplayLine1("PasswordPump");
  DisplayLine2( __DATE__);
  delay(SHOW_SPLASHSCREEN);                                                     // interrupts must be enabled for delay to work
//DisplayLine1("Copyright");                                                    // comment out to save memory
//DisplayLine2("2019 Dan Murphy");                                              // comment out to save memory
//delay(SHOW_COPYRIGHT);

  DisableInterrupts();                                                          // turn off global interrupts

  initSPI();

  if (getResetFlag != MEMORY_INITIALIZED_FLAG) {                                // if memory has never been initialized, initialize it.
    loginFailures = MAX_LOGIN_FAILURES + 1;                                     // so that a condition inside of EVENT_RESET evaluates to true and the reset 
                                                                                // logic is executed. 
    event = EVENT_RESET;                                                        // this is the first time we're turning on the device, initialize memory 
                                                                                // (25LC256 comes with 0x00 in every address space)
    ProcessEvent();                                                             // the reset event will write 0xFF to the location for the reset flag
  };
  
  loginFailures = getLoginFailures;                                             // getLoginFailures returns a byte.
  if (loginFailures == INITIAL_MEMORY_STATE_BYTE ) {                            // if loginFailures has never been written too
    loginFailures = 0;                                                          // set it to zero
    writeLoginFailures();                                                       // and write it to EEprom.
  }

  keyboardFlag = false;                                                         // setup the keyboard flag

  flipOnOff(keyboardFlag,SET_KEYBOARD,KBD_MENU_O_POS);

  showPasswordsFlag = getShowPasswordsFlag;                                     // setup the show passwords flag and menu item. (getShowPasswordsFlag returns byte)
  if (showPasswordsFlag == INITIAL_MEMORY_STATE_BYTE ) {                        // this should never be true because the reset event sets the show passwords 
    showPasswordsFlag = true;                                                   // flag to a value but, for safety, set the show password flag to ON
    writeShowPasswordsFlag();                                                   // and write it to EEprom.
  }
  flipOnOff(showPasswordsFlag,SET_SHOW_PASSWORD,SH_PW_MENU_O_POS);              // set the menu item to Show Passwrd ON or Show Passwrd OFF.

  PCICR |= (1 << PCIE0);                                                        // Setup interrupts for rotary encoder
  PCMSK0 |= (1 << PCINT4) | (1 << PCINT5);                                      

  lastActivityTime = millis();                                                  // establish the start time for when the device is powered up
  authenticated = false;                                                        // we're not authenticated yet!

  headPosition = getListHeadPosition();                                         // read the head of the doubly linked list that sorts by account name
  acctPosition = headPosition;                                                  // initally the current account it the head account
  countAccounts();                                                              // count the number of populated accounts in EEprom

  event = EVENT_SHOW_MAIN_MENU;                                                 // first job is to show the first element of the main menu
  setBlue();                                                                    // not yet authenticated, LED is orange

  EnableInterrupts();                                                                 // Turn on global interrupts
}

void loop() {                                                                   // executes over and over, forever
  encoderButton.loop();                                                         // polling for button press TODO: replace w/ interrupt
  ProcessEvent();                                                               // process any events that might have occurred.
}

void ProcessEvent() {                                                           // processes events as they happen
  if (event != EVENT_NONE) {                                                    // if we have a real event happening...
    DisableInterrupts();                                                        // disable the global interrupts when processing a real event
    lastActivityTime = millis();                                                // bump up the lastActivityTime, we don't reset iterationCount here, not 
                                                                                // necessary and slows responsiveness just a bit
  } else {                                                                      // event == EVENT_NONE
    EnableInterrupts();
    if (++iterationCount == 255) {                                              // we don't want to call millis() every single time through the loop
      iterationCount = 0;                                                       // necessary?  won't we just wrap around?
      if (millis() < (lastActivityTime + MAX_IDLE_TIME)) {                      // check to see if the device has been idle for MAX_IDLE_TIME milliseconds
        return;                                                                 // not time to logout yet and event == EVENT_NONE, so just return.
      } else {
        event = EVENT_LOGOUT;                                                   // otherwise we've been idle for more than MAX_IDLE_TIME, logout.
      }
    } else {                                                                    // iterationCount is < 255
      return;                                                                   // not time to check millis() yet, just return
    }
  }
  
  if (event == EVENT_ROTATE_CW) {                                               // scroll forward through something depending on state...
    if ((STATE_SHOW_MAIN_MENU == machineState) &&
         authenticated                                                      ) { // this prevents navigation away from 'Enter Master Password' when not 
                                                                                // authenticated.
      if (position < MAIN_MENU_ELEMENTS - 1) {                                  // prevent scrolling past the last item on the menu
        position++;
        MenuDown(currentMenu);                                                  // move one position down the current menu
      }
    } else if ((STATE_ENTER_MASTER   == machineState  ) ||
               (STATE_EDIT_ACCOUNT   == machineState  ) ||
               (STATE_EDIT_STYLE     == machineState  ) ||
               (STATE_EDIT_USERNAME  == machineState  ) ||
               (STATE_EDIT_PASSWORD  == machineState  )) {
      if (position < LEN_ALL_CHARS) {
        position++;
      }
      char charToPrint = allChars[position];                                    // TODO: eliminate this assignment / needless charToPrint variable
      ShowChar(charToPrint, enterPosition);
    } else if (STATE_SEND_CREDS_MENU == machineState){
      if (position < SEND_MENU_ELEMENTS - 1) {
        position++;
        MenuDown(currentMenu);
      }
    } else if (STATE_EDIT_CREDS_MENU == machineState){
      if ((position < EDIT_MENU_ELEMENTS - 1) && (acctCount > 0)) {             // we'll only show the edit account options when there's at least one account
        position++;
        MenuDown(currentMenu);
        SwitchRotatePosition(position);
      }
    } else if (STATE_FIND_ACCOUNT == machineState) {     
      uint8_t nextPos = getNextPtr(acctPosition);
      if (nextPos != INITIAL_MEMORY_STATE_BYTE) {
        position = nextPos;
      }
      acctPosition = position;
      readAcctFromEEProm(acctPosition, accountName);
      DisplayLine2(accountName);
    } else if ((STATE_CONFIRM_BACK_EEPROM == machineState ) || 
               (STATE_CONFIRM_RESTORE     == machineState ) ||
               (STATE_CONFIRM_RESET       == machineState ) ||
               (STATE_CONFIRM_DEL_ACCT    == machineState )) {
//             (STATE_CONFIRM_FIX_CORRUPT == machineState ) ||
      if (position == 1) {                                                      // roll through the two options, N and Y
        position = 0;
      } else {
        position = 1; 
      }
      ShowChar(confirmChars[position], POS_Y_N_CONFIRM);
    }
    event = EVENT_NONE;                                                         // to prevent infinite looping
    
  } else if (event == EVENT_ROTATE_CC) {                                        // scroll backward through something depending on state...
    if (STATE_SHOW_MAIN_MENU == machineState) {
      if (position > FIND_ACCOUNT) {                                            // don't show the Master Password menu item after successful authentication
        position--;
        MenuUp(currentMenu);
      }
    } else if ((STATE_ENTER_MASTER  == machineState ) ||
               (STATE_EDIT_ACCOUNT  == machineState ) ||
               (STATE_EDIT_STYLE    == machineState ) ||
               (STATE_EDIT_USERNAME == machineState ) ||
               (STATE_EDIT_PASSWORD == machineState )) {
      if (position > 0) {
        position--;
      }
      char charToPrint = allChars[position];
      ShowChar(charToPrint, enterPosition);
    } else if (STATE_SEND_CREDS_MENU == machineState ){
      if (position > 0) {
        position--;
        MenuUp(currentMenu);
      }
    } else if (STATE_EDIT_CREDS_MENU == machineState){
      if (position > 0) {
        position--;
        MenuUp(currentMenu);
        SwitchRotatePosition(position);
      }
    } else if (STATE_FIND_ACCOUNT == machineState) {

      uint8_t prevPos = getPrevPtr(acctPosition);
      if (prevPos != INITIAL_MEMORY_STATE_BYTE) {
        position = prevPos;
      }
      acctPosition = position;
      readAcctFromEEProm(acctPosition, accountName);
      DisplayLine2(accountName);
    } else if ((STATE_CONFIRM_BACK_EEPROM == machineState ) || 
               (STATE_CONFIRM_RESTORE     == machineState ) ||
               (STATE_CONFIRM_RESET       == machineState ) ||
               (STATE_CONFIRM_DEL_ACCT    == machineState )) {
//             (STATE_CONFIRM_FIX_CORRUPT == machineState ) ||
      if (position == 1) {                                                      // roll through the two options, N and Y
        position = 0;
      } else {
        position = 1; 
      }
      ShowChar(confirmChars[position], POS_Y_N_CONFIRM);
    }
    event = EVENT_NONE;

  } else if (event == EVENT_SHOW_MAIN_MENU) {                                   // show the main menu
    menuNumber = MAIN_MENU_NUMBER;
    int arraySize = 0;
    for (uint8_t i = 0; i < MENU_SIZE; i++) {
      arraySize += sizeof(mainMenu[i]);  
    }
    memcpy(currentMenu, mainMenu, arraySize);
    elements = MAIN_MENU_ELEMENTS;
    machineState = STATE_SHOW_MAIN_MENU;
    if (authenticated) {
      position = FIND_ACCOUNT; 
    } else {
      position = ENTER_MASTER_PASSWORD;
    }
    ShowMenu(position, currentMenu);
    event = EVENT_NONE;

  } else if (event == EVENT_SHOW_EDIT_MENU) {                                   // show the main menu
    menuNumber = EDIT_MENU_NUMBER;
    int arraySize = 0;
    for (uint8_t i = 0; i < MENU_SIZE; i++) {
      arraySize += sizeof(enterMenu[i]);  
    }
    memcpy(currentMenu, enterMenu, arraySize);
    elements = EDIT_MENU_ELEMENTS;
    machineState = STATE_EDIT_CREDS_MENU;
    if (position < 0 || position > (EDIT_MENU_ELEMENTS - 1)) position = 0;      // for safety
    ShowMenu(position, currentMenu);
    readAcctFromEEProm(acctPosition, accountName);
    DisplayLine2(accountName);
    event = EVENT_NONE;

  } else if (event == EVENT_LONG_CLICK) {                                       // jump up / back to previous menu 
    if (STATE_ENTER_MASTER == machineState){
      ReadFromSerial( masterPassword, 
                      MASTER_PASSWORD_SIZE,
                      mainMenu[ENTER_MASTER_PASSWORD]);
      authenticated = authenticateMaster(masterPassword);                       // authenticateMaster writes to masterPassword
      uint8_t i = 0;
      if (authenticated) {
        position = FIND_ACCOUNT;
        machineState = STATE_SHOW_MAIN_MENU;
        ShowMenu(position, currentMenu);
        DisplayLine2("Authenticated");
        event = EVENT_NONE;
      } else {
        if (loginFailures > MAX_LOGIN_FAILURES) {
          event = EVENT_RESET;                                                  // factory reset after 10 failed attempts to enter master password!
        } else {  
          position = 0;
          machineState = STATE_SHOW_MAIN_MENU;
          ShowMenu(position, currentMenu);
          char buffer[4];
          itoa(loginFailures, buffer, 10);                                      // convert login failures to a string and put it in buffer.
          strcpy(line2DispBuff, buffer);
          strcat(line2DispBuff, " failure(s)");
          DisplayBuffer();
          event = EVENT_NONE;
        }
      }
    } else if (STATE_EDIT_ACCOUNT == machineState) {                            // Accept input for the account name
      ProcessAttributeInput(  accountName,
                              ACCOUNT_SIZE,
                              enterMenu[EDIT_ACCT_NAME],
                              EDIT_USERNAME,
                              true,
                              GET_ADDR_ACCT(acctPosition) );
    } else if (STATE_EDIT_USERNAME == machineState) {                           // Accept input for the username
      ProcessAttributeInput(  username,
                              USERNAME_SIZE,
                              enterMenu[EDIT_USERNAME],
                              EDIT_PASSWORD,
                              false,
                              GET_ADDR_USER(acctPosition) );
    } else if (STATE_EDIT_PASSWORD == machineState) {                           // Accept input for the password
      ProcessAttributeInput(  password,
                              PASSWORD_SIZE,
                              enterMenu[EDIT_PASSWORD],
                              EDIT_STYLE,
                              false,
                              GET_ADDR_PASS(acctPosition) );
    } else if (STATE_EDIT_STYLE == machineState) {                              // Accept input for the style
      ReadFromSerial(style, STYLE_SIZE, enterMenu[EDIT_STYLE]);
      eeprom_write_bytes(GET_ADDR_STYLE(acctPosition), style, STYLE_SIZE);
      position = EDIT_ACCT_NAME;
      event = EVENT_SHOW_EDIT_MENU;
    } else if (STATE_EDIT_CREDS_MENU == machineState){                          // go back to send creds menu or main menu,depending on how you got here.
      if (!addFlag) {
        switchToSendCredsMenu();
      } else {
        event = EVENT_SHOW_MAIN_MENU;
      }
    } else if (STATE_SEND_CREDS_MENU == machineState){
      switchToFindAcctMenu();
    } else if (STATE_FIND_ACCOUNT == machineState){                             // long click after selecting an account
      event = EVENT_SHOW_MAIN_MENU;
      BlankLine2();
    } else if ((STATE_CONFIRM_BACK_EEPROM   == machineState) ||
               (STATE_CONFIRM_RESTORE       == machineState) ||
               (STATE_CONFIRM_RESET         == machineState)) {
//             (STATE_CONFIRM_FIX_CORRUPT   == machineState) || 
      event = EVENT_SHOW_MAIN_MENU;
    } else if (STATE_CONFIRM_DEL_ACCT == machineState) {
      switchToSendCredsMenu();
    } else {
      event = EVENT_SHOW_MAIN_MENU;                                             // if any other state show main menu (e.g just after EVENT_RESET)
    }

  } else if (event == EVENT_SINGLE_CLICK) {
    if (STATE_SHOW_MAIN_MENU == machineState) {
      switch(position) {
        case ENTER_MASTER_PASSWORD:                                             // Enter master password
          machineState = STATE_ENTER_MASTER;
          position = DEFAULT_ALPHA_EDIT_POS;                                    // puts the position of the rotary encoder over 'A' for quicker password  entry
          enterPosition = 0;
          char charToPrint[2];
          charToPrint[0] = allChars[enterPosition];
          charToPrint[1] = NULL_TERM;
          DisplayLine2(charToPrint);
          if (keyboardFlag) {
            Serial.begin(BAUD_RATE);
            while(!Serial);
          }
          event = EVENT_NONE;
          break;
        case FIND_ACCOUNT:                                                      // Find account
          addFlag = false;
          switchToFindAcctMenu();
          break;
        case LOGOUT:                                                            // Logout  DEFECT: why is this being skipped over
          event = EVENT_LOGOUT;
          break;
        case SET_KEYBOARD:
          keyboardFlag = !keyboardFlag;
          flipOnOff(keyboardFlag,SET_KEYBOARD,KBD_MENU_O_POS);
          DisplayLine1(mainMenu[SET_KEYBOARD]);
          event = EVENT_NONE;
          break;
        case SET_SHOW_PASSWORD:
          showPasswordsFlag = !showPasswordsFlag;
					writeShowPasswordsFlag();
          flipOnOff(showPasswordsFlag,SET_SHOW_PASSWORD,SH_PW_MENU_O_POS);      // set the menu item to Show Passwrd ON or Show Passwrd OFF.
          DisplayLine1(mainMenu[SET_SHOW_PASSWORD]);
          event = EVENT_NONE;
          break;
        case BACKUP_EEPROM:                                                     // Backup EEprom
          event = EVENT_BACKUP;
          break;
        case BACKUP_ALL:                                                        // Send all creds out through the keyboard for capture in a text editor
          event = EVENT_BACKUP_TO_FILE;
          break;
        case RESTORE_BACKUP:                                                    // Restore the backup EEprom to the primary
          event = EVENT_RESTORE;
          break;
//      case FIX_CORRUPT_LIST:                                                  // Fix any corruption in the linked list
//        event = EVENT_FIX_CORRUPTION;
//        break;
        case FACTORY_RESET:                                                     // Reset
          event = EVENT_RESET;
          break;
        case ADD_ACCOUNT:                                                       // Add account
          addFlag = true;                                                       // necessary for when we return to the main menu
          acctPosition = getNextFreeAcctPos();                                  // get the position of the next EEprom location for account name marked empty.
          if (acctPosition != INITIAL_MEMORY_STATE_BYTE) {
            username[0] = NULL_TERM;
            password[0] = NULL_TERM;
            strcpy(accountName,"Add Account");
            DisplayLine2(accountName);
            acctCount++;
          } else {
            DisplayLine2("No space");
            break;
          }
          switchToEditMenu();
          break;
        default:                                                                // deleting this line and the following line ADDS 14 bytes to program storage space!
          break;
      }
      if (event == EVENT_SINGLE_CLICK) {                                        // stop the infinite loop of single clicks
        event = EVENT_NONE;
      }
    } else if (STATE_FIND_ACCOUNT == machineState) {                            // Go to the send menu 
      switchToSendCredsMenu();
    } else if (STATE_EDIT_CREDS_MENU == machineState) {
      enterPosition = 0;
      switch(position) {
        case EDIT_ACCT_NAME:                                                    // Enter account name
          if (addFlag) {                                                        // Prevent editing the account name except when adding the credentials. Editing
            EditAttribute(STATE_EDIT_ACCOUNT, DEFAULT_ALPHA_EDIT_POS);          // the account name in place doesn't work, it corrupts the linked list.
          } else {
            DisplayLine2("No edit acct name");
            event = EVENT_NONE;
          }
          break; 
        case EDIT_USERNAME:                                                    // Enter username     
          EditAttribute(STATE_EDIT_USERNAME, DEFAULT_ALPHA_EDIT_POS);
          break; 
        case EDIT_PASSWORD:                                                    // Enter Password   
          EditAttribute(STATE_EDIT_PASSWORD, DEFAULT_ALPHA_EDIT_POS);
          break; 
        case EDIT_STYLE:
          EditAttribute(STATE_EDIT_STYLE, DEFAULT_STYLE_EDIT_POS);
          break; 
        case GENERATE_PASSWORD:                                                // Automatic UUID enter password 
          machineState = STATE_EDIT_PASSWORD;                                 // pretend we're entering the password
          setUUID(password, PASSWORD_SIZE, true);                             // put a UUID in the password char array
          BlankLine2();
          event = EVENT_LONG_CLICK;                                           // and trigger long click to write the password to eeprom.
          break;
      }
    } else if (STATE_EDIT_ACCOUNT == machineState) {
      enterAttributeChar(accountName, false);
    } else if (STATE_EDIT_USERNAME == machineState) {
      enterAttributeChar(username, false);
    } else if (STATE_EDIT_PASSWORD == machineState) {
      enterAttributeChar(password, true);
    } else if (STATE_EDIT_STYLE == machineState) {
      enterAttributeChar(style, false);
    } else if (STATE_SEND_CREDS_MENU == machineState){
      setPurple();
      event = EVENT_NONE;
      switch(position) {
         case SEND_USER_AND_PASSWORD:                                                                
            sendUsernameAndPassword();                                          // Send the username and password
            DisplayLine2("Sent user/pass");
            break; 
         case SEND_PASSWORD:                                                    
            sendPassword();                                                     // Send the password
            sendRTN();                                                          // Send the carriage return
            DisplayLine2("Sent password");
            break;
         case SEND_USERNAME:                                                    
            sendUsername();                                                     // Send the username
            DisplayLine2("Sent username");
            break;
         case SEND_PASSWORD_NO_RET:
            sendPassword();                                                     // Send the password
            DisplayLine2("Sent password");
            break;
         case SEND_ACCOUNT:                                                     // Send the account name
            sendAccount();
            DisplayLine2("Sent acct name");
            break;
         case EDIT_ACCOUNT:                                                     // Show the enter account menu
            switchToEditMenu();
            break;
         case DELETE_ACCOUNT:                                                   // Delete account
            event = EVENT_DELETE_ACCT;
            break;
      }
      setGreen();
    } else if (STATE_ENTER_MASTER == machineState) {
      masterPassword[enterPosition] = allChars[position];
      masterPassword[enterPosition + 1] = NULL_TERM;                            // push the null terminator out ahead of the last char in the string
      if (showPasswordsFlag) {
        line2DispBuff[enterPosition] = allChars[position];
      } else {
        line2DispBuff[enterPosition] = '*';
      }
      line2DispBuff[enterPosition + 1] = NULL_TERM;                             // push the null terminator out ahead of the last char in the string
      DisplayBuffer();
      if (enterPosition < DISPLAY_BUFFER_SIZE) enterPosition++;                 // don't increment enterPosition beyond the space that's allocated for the associated array
      event = EVENT_NONE;
    } else if (STATE_CONFIRM_BACK_EEPROM == machineState) {
      if (confirmChars[position] == 'Y') {
        CopyChip(false);
      }
      event = EVENT_SHOW_MAIN_MENU;
    } else if (STATE_CONFIRM_RESTORE == machineState) {
      if (confirmChars[position] == 'Y') {
        CopyChip(true);
      }
      event = EVENT_SHOW_MAIN_MENU;
//  } else if (STATE_CONFIRM_FIX_CORRUPT == machineState) {
//    if (confirmChars[position] == 'Y') {
//      FixCorruptLinkedList();
//    }
//    event = EVENT_SHOW_MAIN_MENU;
    } else if (STATE_CONFIRM_RESET == machineState) {
      if (confirmChars[position] == 'Y') {
        FactoryReset();
      }
      event = EVENT_SHOW_MAIN_MENU;
    } else if (STATE_CONFIRM_DEL_ACCT == machineState) {
      if (confirmChars[position] == 'Y') {
        deleteAccount(acctPosition);
      }
      switchToFindAcctMenu();
    }

  } else if (event == EVENT_BACKUP) {
    ConfirmChoice(STATE_CONFIRM_BACK_EEPROM);

  } else if (event == EVENT_RESTORE) {
    ConfirmChoice(STATE_CONFIRM_RESTORE);

//} else if (event == EVENT_FIX_CORRUPTION) {
//  ConfirmChoice(STATE_CONFIRM_FIX_CORRUPT);

  } else if (event == EVENT_RESET) {
    ConfirmChoice(STATE_CONFIRM_RESET);

  } else if (event == EVENT_DELETE_ACCT) {
    ConfirmChoice(STATE_CONFIRM_DEL_ACCT);

  } else if (event == EVENT_BACKUP_TO_FILE) {
    sendAll();
    event = EVENT_NONE;

  } else if (event == EVENT_LOGOUT) {                                           // TODO: you need to be logged in to logout, check for authentication here
    if(authenticated) {    
      DisplayLine2("Logged out");
      position = 0;
      acctPosition = 0;
      uint8_t i;
      while (i < MASTER_PASSWORD_SIZE) masterPassword[i++] = NULL_TERM;
      authenticated = false;                                                    // we're no longer authenticated, we need to re-enter the master password
      loginFailures = 0;
      writeLoginFailures();
      setBlue();
      event = EVENT_SHOW_MAIN_MENU;
    } else {
      DisplayLine2("Not logged in");
      event = EVENT_SHOW_MAIN_MENU;
    }
  }
}

void ProcessAttributeInput( uint8_t *attributeName, 
                            uint8_t attributeSize, 
                            char    *menuName, 
                            uint8_t nextPosition,
                            uint8_t acctFlag,
                            uint16_t address          ) {
  ReadFromSerial(attributeName, attributeSize, menuName);
  uint8_t pos = 0;
  while (attributeName[pos++] != NULL_TERM);                                    // make sure the account name is 16 chars long, pad with NULL_TERM
  while (pos < attributeSize) attributeName[pos++] = NULL_TERM;                 // "           "              "
  uint8_t buffer[attributeSize];
  setKey(acctPosition);
  encrypt32Bytes(buffer, attributeName);
  eeprom_write_bytes(address, buffer, attributeSize);
  if (acctFlag) writePointers(acctPosition, attributeName);                     // insert the account into the linked list by updating prev and next pointers.
  position = nextPosition;
  event = EVENT_SHOW_EDIT_MENU;   
}

void setKey(uint8_t pos) {
  uint8_t key[MASTER_PASSWORD_SIZE + CRED_SALT_SIZE];                           // key size is 16
  readCredSaltFromEEProm(pos, key);                                             // puts the salt in the first part of key
  if(key[0] == INITIAL_MEMORY_STATE_BYTE) {                                     // if the salt is missing, add it
    setCredSalt(key, CRED_SALT_SIZE);                                           // calculate the salt and put it in key
    write_eeprom_array(GET_ADDR_CRED_SALT(pos), key, CRED_SALT_SIZE);           // write the new salt value to EEprom
  }
  memcpy(key + CRED_SALT_SIZE, masterPassword, MASTER_PASSWORD_SIZE);           // append the master password to the key to set the encryption key
  aes.setKey(key, MASTER_PASSWORD_SIZE + CRED_SALT_SIZE);                       // set the key for aes to equal the salt||un-hashed entered master password
}

void enterAttributeChar(uint8_t *attribute, uint8_t passwordFlag) {
  attribute[enterPosition] = allChars[position];
  attribute[enterPosition + 1] = NULL_TERM;                                     // push the null terminator out ahead of the last char in the string
  if (!showPasswordsFlag && passwordFlag) {                                     // mask the password being entered if showPasswordsFlag is OFF
    line2DispBuff[enterPosition] = '*';
  } else {
    line2DispBuff[enterPosition] = allChars[position];
  }
  line2DispBuff[enterPosition + 1] = NULL_TERM;                                 // push the null terminator out ahead of the last char in the string
  DisplayBuffer();
  if ((enterPosition + 1) < (DISPLAY_BUFFER_SIZE - 1)) enterPosition++;         // don't increment enterPosition beyond the space that's allocated for the associated array
  event = EVENT_NONE;
}

void EditAttribute(uint8_t aState, uint8_t pos) {
  char charToPrint[2];
  charToPrint[0] = allChars[enterPosition];
  charToPrint[1] = NULL_TERM;                                                   // TODO: this shouldn't be necessary
  machineState = aState; 
  position = pos;                                                               // setting postion for starting input
  DisplayLine2(charToPrint);                                                    // echo the char
  event = EVENT_NONE;
  if (keyboardFlag) {
    Serial.begin(BAUD_RATE);
    while(!Serial);                                                             // waits for an active serial connection to be established by the PC (i.e., for
  }                                                                             // the serial port to be opened by a piece of software)
}

void ConfirmChoice(int state) {                                                 // display the menu item that confirms execution of some destructive function
  machineState = state;                                                         // set machineState to the passed in state
  DisplayLine2("Are you sure? ");
  position = 0;                                                                 // confirmChars[0] = 'N'
  ShowChar(confirmChars[position], POS_Y_N_CONFIRM);                            // default choice is always 'N'; No.
  event = EVENT_NONE;                                                           // wait for input from the rotary encoder or short or long button press
}

void ReadFromSerial(char *buffer, uint8_t size, char *prompt) {                 // get input from the keyboard
  if(keyboardFlag) {                                                            // but only if the keyboard is enabled
    EnableInterrupts();                                                         // Serial uses global interrupts, they must be enabled
    Serial.println("");Serial.println(prompt);                                  // display the name of the element to be collected to the end user via serial terminal
    uint8_t serialCharCount = Serial.available();
    if (serialCharCount > size) serialCharCount = size;                         // ensure we don't read more bytes than that which we can accomodate in the buffer
    if (serialCharCount > 0) {
      for (uint8_t i = 0; i < (serialCharCount - 1); i++) {
        buffer[i] = Serial.read();                                              // read values input via serial monitor
        Serial.write(buffer[i]);                                                // echo the character to serial monitor or PuTTY
      }
      buffer[serialCharCount - 1] = NULL_TERM;                                  // substitute the final \n with \0.
      //Serial.read();                                                          // read the last byte in the buffer, throw it out.
      while (Serial.available()) Serial.read();                                 // substituted the above method with this method when we started making sure 
                                                                                // that we don't read past the end of the passed buffer. Extra bytes are tossed
    }
    Serial.end();
    DisableInterrupts();                                                        // disable the global interrupts when done with Serial
  }
}

void switchToEditMenu(){
  menuNumber = EDIT_MENU_NUMBER;
  elements = EDIT_MENU_ELEMENTS;
  int arraySize = 0;
  for (uint8_t i = 0; i < MENU_SIZE; i++) {
    arraySize += sizeof(enterMenu[i]);  
  }
  memcpy(currentMenu, enterMenu, arraySize);
  elements = EDIT_MENU_ELEMENTS;
  position = EDIT_ACCT_NAME;
  machineState = STATE_EDIT_CREDS_MENU;
  ShowMenu(position, currentMenu);
  if (!addFlag) {
    readAcctFromEEProm(acctPosition, accountName);
    DisplayLine2(accountName);
  }
  event = EVENT_NONE;
}

void switchToSendCredsMenu() {
  //acctPosition = position;  //misbehaving
  menuNumber = SEND_MENU_NUMBER;
  elements = SEND_MENU_ELEMENTS;
  int arraySize = 0;
  for (uint8_t i = 0; i < MENU_SIZE; i++) {
    arraySize += sizeof(sendMenu[i]);  
  }
  memcpy(currentMenu, sendMenu, arraySize);
  elements = SEND_MENU_ELEMENTS;
  position = SEND_USER_AND_PASSWORD;
  machineState = STATE_SEND_CREDS_MENU;
  ShowMenu(position, currentMenu);
  readAcctFromEEProm(acctPosition, accountName);
  DisplayLine2(accountName);
  event = EVENT_NONE;
}

void switchToFindAcctMenu() {
  machineState = STATE_FIND_ACCOUNT;
  position = headPosition; 
  acctPosition = headPosition;
//  Serial.println(position);
  ShowMenu(FIND_ACCOUNT, mainMenu);
  readAcctFromEEProm(position, accountName);
//  Serial.println((char *) accountName);
  DisplayLine2(accountName);
  event = EVENT_NONE;
}

void SwitchRotatePosition(uint8_t pos) {
  switch(pos) {                                                                 // decide what to print on line 2 of the display
    case EDIT_ACCT_NAME:
      if (!addFlag) readAcctFromEEProm(acctPosition, accountName);
      DisplayLine2(accountName);
      break;
    case EDIT_USERNAME:
      if (!addFlag) readUserFromEEProm(acctPosition, username);
      DisplayLine2(username);
      break;
    case EDIT_PASSWORD:
      if (!addFlag) readPassFromEEProm(acctPosition, password);
      if (showPasswordsFlag) {
        DisplayLine2(password);
      } else {
        BlankLine2();
      }
      break;
    case EDIT_STYLE:
      if (!addFlag) readStyleFromEEProm(acctPosition, style);
      DisplayLine2(style);
      break;
    case GENERATE_PASSWORD:
      BlankLine2();
      break;
  }
}

void FactoryReset() {
  if (authenticated || (loginFailures > MAX_LOGIN_FAILURES)) {                  // TODO: re-enter master password here to authorize creds reset
    acctCount = 0;
    acctPosition = 0;
    headPosition = 0;
    tailPosition = 0;
    uint8_t i;
    while (i < MASTER_PASSWORD_SIZE) masterPassword[i++] = NULL_TERM;           // write over the master password in memory as soon as possible
    authenticated = false;                                                      // we're no longer authenticated, we need to re-enter the master password
    DisplayLine1("Initializing...");
    InitializeEEProm();                                                         // sets all of memory = INITIAL_MEMORY_STATE_BYTE, 0xFF/255/0b11111111
    InitializeIntEEProm();                                                      // initialize internal EEprom
    writeResetFlag(MEMORY_INITIALIZED_FLAG);                                    // setting the last byte in external EEprom to 0x01 signals that all other 
                                                                                // memory has been initialized to INITIAL_MEMORY_STATE_BYTE and that Initialize
                                                                                // doesn't need to execute at startup.
    setBlue();                                                                  // we are no longer logged in
    loginFailures = 0;                                                          // set login failures back to zero, this also serves as a flag to indicate if 
                                                                                // it's the first power on
    writeLoginFailures();                                                       // write login failure count back to EEprom
    showPasswordsFlag = true;                                                   // to match the out of box setting (true / 255)
    writeShowPasswordsFlag();                                                   // write show passwords flag back to EEprom
    flipOnOff(showPasswordsFlag,SET_SHOW_PASSWORD,SH_PW_MENU_O_POS);            // set the menu item to Show Passwrd ON or Show Passwrd OFF.
//  keyboardFlag = false;
//  writeKeyboardFlag();
//  flipOnOff(keyboardFlag,SET_KEYBOARD,KBD_MENU_O_POS);                        // set the menu item accordingly
    accountName[0] = NULL_TERM;
    password[0] = NULL_TERM;
    username[0] = NULL_TERM;
    DisplayLine2("All creds erased");
  } else { 
    DisplayLine2("Not logged in");
  }
}

//- Interrupt Service Routines

ISR(PCINT0_vect) {                                                              // Interrupt service routine for rotary encoder
  unsigned char result = rotaryEncoder.process();   
  if (result == DIR_CW) {                                                       // rotated encoder clockwise
    event = EVENT_ROTATE_CW;
  }
  else if (result == DIR_CCW) {                                                 // rotated encoder counter clockwise
    event = EVENT_ROTATE_CC;
  }
}

//- Button

void buttonReleasedHandler(Button2& btn) {
  if(btn.wasPressedFor() > LONG_CLICK_LENGTH) {
    event = EVENT_LONG_CLICK;
  } else {
    event = EVENT_SINGLE_CLICK;
  }
}

//- Delete Account

void deleteAccount(uint8_t position) {
  DisplayLine2("Erasing creds");

  uint8_t prevPosition = getPrevPtr(position);                                  // get the previous account position from the linked list
  uint8_t nextPosition = getNextPtr(position);                                  // get the next account position from the linked list

  if(prevPosition != INITIAL_MEMORY_STATE_BYTE) {                               // if we're not already the head position
    writeNextPtr(prevPosition, nextPosition);                                   // write the next account position into the next account pointer of the previous position
  } else {
    headPosition = nextPosition;                                                // we're deleting the head, make the next element the new head
  }
  if(nextPosition != INITIAL_MEMORY_STATE_BYTE) {                               // if we're not already the tail position
    writePrevPtr(nextPosition, prevPosition);                                   // write the previous account position into the previous account pointer of the next position
  } else {
    tailPosition = prevPosition;                                                // we're deleting the dail, make the previous element the new tail
  }

  writeNextPtr(position, INITIAL_MEMORY_STATE_BYTE);                            // set the next pointer for this position to 255
  writePrevPtr(position, INITIAL_MEMORY_STATE_BYTE);                            // set the previous pointer for this position to 255
  
  uint8_t emptyPassword[PASSWORD_SIZE];
  for (uint8_t i = 0; i < PASSWORD_SIZE; i++) {
    emptyPassword[i] = NULL_TERM;                                               // to completely overwrite the password in EEProm
  }
  byte allBitsOnArray[2];
  allBitsOnArray[0] = INITIAL_MEMORY_STATE_BYTE;                                // this makes the account name free/empty/available
  allBitsOnArray[1] = NULL_TERM;
  byte firstNullTermArray[1];
  firstNullTermArray[0] = NULL_TERM;                                            // equivalent to ""
  writeAllToEEProm( allBitsOnArray,                                             // account:  "-1\0", the -1 signals that the position is free/empty/available.
                    firstNullTermArray,                                         // username: "\0", so when it is read it will come back empty
                    emptyPassword,                                              // password: "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", to overwrite the entire pw
                    position                );                                  // position to be deleted
  accountName[0] = NULL_TERM;
  memcpy(password, emptyPassword, PASSWORD_SIZE);
  username[0] = NULL_TERM;

  acctCount--;
  acctPosition = headPosition;
  DisplayLine2("Creds erased");
}

//- UUID Generation

void setUUID(uint8_t *password, uint8_t size, uint8_t appendNullTerm) {
  for (uint8_t i = 0; i < size; i++) {
    password[i] = random(33,126);                                               // maybe we should use allChars here instead? We're generating PWs w/ chars that we can't input...
                                                                                // 32 = space, 127 = <DEL>, so we want to choose from everything in between.
    if (appendNullTerm) password[size - 1] = NULL_TERM;
  }
}

void setCredSalt(uint8_t *credSalt, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    credSalt[i] = random(0,255);                                                
  }
}

//- Keyboard Functions

void sendAccount() {
  readAcctFromEEProm(acctPosition, accountName);                                // read the account name from EEProm
  Keyboard.begin();                                                             // TODO: can we do a <CTL><A> <BS> here first?  That will clear out pre-populated usernames.
  uint8_t pos = 0;
  while ((char) accountName[pos] != NULL_TERM) {
    Keyboard.write((char) accountName[pos++]);                                  // using Keyboard.println() here doesn't work; the Keyboard.println function appears to generate corrupt output (e.g. ".com" == ".comcast")
  }
  Keyboard.end();
}

void sendUsername() {
  readUserFromEEProm(acctPosition, username);                                   // read the username from EEProm
  char usernameChar[USERNAME_SIZE];
  memcpy(usernameChar,username,USERNAME_SIZE);
  Keyboard.begin();                                                             // TODO: can we do a <CTL><A> <BS> here first?  That will clear out pre-populated usernames.
  Keyboard.print(usernameChar);                                                 // type the username through the keyboard, no carriage return
  Keyboard.press(TAB_KEY);                                                      // send <TAB>
  Keyboard.release(TAB_KEY);
  Keyboard.end();
}

void sendPassword() {                                                           // TODO: can we do a <CTL><A> <BS> here first? That will clear out pre-populated passwords.
  readPassFromEEProm(acctPosition, password);                                   // read the password from EEProm
  char passwordChar[PASSWORD_SIZE];
  memcpy(passwordChar,password,PASSWORD_SIZE);
  Keyboard.begin();
  Keyboard.print(passwordChar);                                                 // type the password through the keyboard, no carriage return
  Keyboard.end();
}

void sendRTN() {
  Keyboard.begin();
  Keyboard.println("");                                                         // send a carriage return through the keyboard
  Keyboard.end();
}

void sendUsernameAndPassword() {
  readAcctFromEEProm(acctPosition, accountName);                                // TODO: is the read from EEprom necessary at this point?
  readUserFromEEProm(acctPosition, username);                                   // read the username from EEProm
  char usernameChar[USERNAME_SIZE];                                             // TODO: eliminate by casting
  memcpy(usernameChar,username,USERNAME_SIZE);
  readPassFromEEProm(acctPosition, password);                                   // read the password from EEProm
  char passwordChar[PASSWORD_SIZE];                                             // TODO: eliminate by casting
  memcpy(passwordChar,password,PASSWORD_SIZE);
  readStyleFromEEProm(acctPosition, style);                                     // read the style from EEprom
  Keyboard.begin();
  uint8_t i = 0;
  while (usernameChar[i++] != NULL_TERM) {
    Keyboard.write(usernameChar[i - 1]);                                        // seems to be a problem only with single character usernames.
  }
  if ((strcmp(style, "0") == 0              ) ||
      (style[0] == INITIAL_MEMORY_STATE_CHAR)   ) {                             // this should make <CR> the default
    Keyboard.println("");                                                       // send <CR> through the keyboard
  } else {
    Keyboard.press(TAB_KEY);                                                    // if style isn't default or "0" then send <TAB>
    Keyboard.release(TAB_KEY);
  }
  EnableInterrupts();
  delay(UN_PW_DELAY);
  DisableInterrupts();
  Keyboard.println(passwordChar);                                               // type the password through the keyboard
  Keyboard.end();
}

void sendAll() {                                                                // this is the function we use to backup all of the accountnames, usernames and passwords
  DisplayLine2("Backup to file");
  setPurple();
  acctPosition = headPosition;
  while (acctPosition != INITIAL_MEMORY_STATE_BYTE) {
    sendAccount();
    Keyboard.println("");                                                       // send <CR> through the keyboard
    sendUsername();
    Keyboard.println("");                                                       // send <CR> through the keyboard
    sendPassword();
    Keyboard.begin();
    Keyboard.println("");
    Keyboard.println("");                                                       // place a carriage return between each account
    Keyboard.end();
    acctPosition = getNextPtr(acctPosition);
  }
  setGreen();
  DisplayLine2("Done Backup");
}

//- Display Control

void DisplayLine1(char* lineToPrint) {
  strncpy(line1DispBuff, lineToPrint, DISPLAY_BUFFER_SIZE);
  line1DispBuff[DISPLAY_BUFFER_SIZE - 1] = NULL_TERM;                           // important in the case where length of lineToPrint exceeds 32.
  DisplayBuffer();
}

void DisplayLine2(char* lineToPrint) {
  strncpy(line2DispBuff, lineToPrint, DISPLAY_BUFFER_SIZE);
  line2DispBuff[DISPLAY_BUFFER_SIZE - 1] = NULL_TERM;                           // important in the case where length of lineToPrint exceeds 32.
  DisplayBuffer();
}

void BlankLine1() {
  strncpy(line1DispBuff,spaceFilled16,DISPLAY_BUFFER_SIZE);
  DisplayBuffer();
}

void BlankLine2() {
  strncpy(line2DispBuff,spaceFilled16,DISPLAY_BUFFER_SIZE);
  DisplayBuffer();
}

void DisplayBuffer() {
  oled.clear();
  oled.println(line1DispBuff);
  oled.println(line2DispBuff);
}

void ShowMenu(uint8_t position, char **menu) {
  DisplayLine1(menu[position]);
}

void MenuUp(char **menu) { 
  if (position > -1) {
    ShowMenu(position, menu);
  }
}

void MenuDown(char **menu){ 
  if (position < elements) {
    ShowMenu(position, menu);
  }
}

void ShowChar(char charToShow, uint8_t  pos) {
  DisplayLine2(line2DispBuff);
  char charToPrint[2];
  charToPrint[0] = charToShow;
  charToPrint[1] = NULL_TERM;
  oled.setCol((NO_LED_LIB_CHAR_WIDTH_PIX * pos) + FIXED_CHAR_SPACING);
  oled.write(charToPrint);
}

void flipOnOff(uint8_t flag, uint8_t pos, uint8_t startInx) {
  if (!flag) {
    mainMenu[pos][startInx]   = 'F';
    mainMenu[pos][startInx+1] = 'F';
    mainMenu[pos][startInx+2] = NULL_TERM;
  } else {
    mainMenu[pos][startInx]   = 'N';
    mainMenu[pos][startInx+1] = NULL_TERM;
  }
}

//- RGB LED

void setPurple() {
  setColor(170, 0, 255);                                                        // Purple Color
}

void setRed(){
  setColor(255, 0, 0);                                                          // Red Color
}

void setGreen(){
  setColor(0, 255, 0);                                                          // Green Color
}


void setYellow(){
  setColor(255, 255, 0);                                                        // Yellow Color
}


void setBlue(){
  setColor(0, 0, 255);                                                          // Blue Color
}

void setColor(uint8_t  redValue, uint8_t  greenValue, uint8_t  blueValue) {
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
}

//- Encryption

boolean authenticateMaster(uint8_t *enteredPassword) {                          // verify if the master password is correct here
  uint8_t eepromMasterHash[HASHED_MASTER_PASSWORD_SZ];                          // buffer for the hashed master password; salt||masterPassword
  uint8_t enteredMasterHash[HASHED_MASTER_PASSWORD_SZ];                         // holds the unhashed master password after some processing
  uint8_t salt[MASTER_PASSWORD_SIZE];                                           // will hold the salt, read from EEprom or calculated
  uint8_t pos = 0;
  while (enteredPassword[pos++] != NULL_TERM);                                  // make sure the unencrypted password is 16 chars long
  while (pos < (MASTER_PASSWORD_SIZE - 1)) enteredPassword[pos++] = NULL_TERM;  // "           "              " , right padded w/ NULL terminator
  enteredPassword[MASTER_PASSWORD_SIZE - 1] = NULL_TERM;                        // NULL_TERM in index 13 no matter what (TODO: is this necessary?)
  //Serial.print("enteredPassword: ");Serial.println((char *)enteredPassword);
//DisableInterrupts();                                                        // disable interrupts
  byte aByte = EEPROM.read(GET_ADDR_MASTER_HASH);
//EnableInterrupts();                                                         // re-enable interrupts (TODO: defer this?)
  if (aByte == INITIAL_MEMORY_STATE_BYTE){                                      // first time, we need to write instead of read
    setUUID(salt, MASTER_PASSWORD_SIZE, false);                                 // generate a random salt
    //Serial.print("calced salt: ");Serial.println((char *)salt);
    eeprom_write_int_bytes(GET_ADDR_SALT, salt, MASTER_PASSWORD_SIZE);          // save the salt to EEprom
    memcpy(eepromMasterHash, salt, MASTER_PASSWORD_SIZE);                       // copy salt into the hashed master password variable
    //Serial.print("eepromMasterHash w/ salt: ");Serial.println((char *)eepromMasterHash);
    memcpy(eepromMasterHash + MASTER_PASSWORD_SIZE,                             // concatinate the salt and the master password
           enteredPassword, 
           MASTER_PASSWORD_SIZE                          );
    //Serial.print("eepromMasterHash w/ all: ");Serial.println((char *)eepromMasterHash);
    //Serial.print("eepromMasterHash 2nd half: ");Serial.println((char *) eepromMasterHash + MASTER_PASSWORD_SIZE);
    sha256Hash(eepromMasterHash);                                               // hash the master password in place; pass in 32, get back 16
    //Serial.print("hashed eepromMasterHash: ");Serial.println((char *)eepromMasterHash);
    eeprom_write_int_bytes(GET_ADDR_MASTER_HASH,                                // only write the first 16 bytes of the hashed master password
                           eepromMasterHash, 
                           HASHED_MASTER_PASSWORD_SZ);                          // write the (hased) master password to EEprom
    setGreen();                                                                 // turn the RGB green to signal the correct password was provided
  } else {                                                                      // (buf != INITIAL_MEMORY_STATE_BYTE) | (ch != INITIAL_MEMORY_STATE_CHAR)
    eeprom_read_int_string(GET_ADDR_MASTER_HASH,                                // read hashed master password from EEprom
                           eepromMasterHash,                                    // to compare against the hash of the salt||entered password.
                           HASHED_MASTER_PASSWORD_SZ);
    //Serial.print("eepromMasterHash: ");Serial.println((char *)eepromMasterHash);
    eeprom_read_int_string(GET_ADDR_SALT,                                       // read salt from EEprom
                           salt, 
                           MASTER_PASSWORD_SIZE);
    //Serial.print("salt: ");Serial.println((char *)salt);
    memcpy(enteredMasterHash, salt, MASTER_PASSWORD_SIZE);                      // copy salt into the hashed master password variable
    //Serial.print("enteredMasterHash: ");Serial.println((char *)enteredMasterHash);
    memcpy(enteredMasterHash + MASTER_PASSWORD_SIZE,                            // concatinate the salt and the master password
           enteredPassword,                                                     // entered password
           MASTER_PASSWORD_SIZE                          );
    //Serial.print("enteredMasterHash: ");Serial.println((char *)enteredMasterHash);
    sha256Hash(enteredMasterHash);                                              // hash the master salt||entered password
    //Serial.print("enteredMasterHash: ");Serial.println((char *)enteredMasterHash);
    if (0 == memcmp(enteredMasterHash,
                    eepromMasterHash,
                    HASHED_MASTER_PASSWORD_SZ)) {                               // entered password hash matches master password hash, authenticated
      setGreen();                                                               // turn the RGB green to signal the correct password was provided
      loginFailures = 0;                                                        // reset loginFailues to zero
      writeLoginFailures();                                                     // record loginFailures in EEprom
                                                                                // encrypt a word using the master password as the key
    } else {                                                                    // failed authentication
// Begin: decoy password comment                                                // Following section commented out because decoy logic needs to change to accomodate a hashed master password
//        if (0 == strcmp(password,strcat(buff,"FR"))) {                        // check for decoy password; masterPassword + "FR".
//          loginFailures = MAX_LOGIN_FAILURES + 2;                             // to turn this functionality back on we'd need to store a hashed version of masterPassword + "FR"
//          event = EVENT_RESET;                                                // in EEprom for comparison to the input password.
//          ProcessEvent();
//        } else {
// End: decoy password comment
      setRed();                                                                 // turn the RGB red to signal the incorrect password was provided
      loginFailures++;
      writeLoginFailures();
      return false;
    }
  }
//aes.setKey(enteredPassword, MASTER_PASSWORD_SIZE);                            // set the key for aes to equal the un-hashed entered master password
//pos = 0;
//while (pos < (MASTER_PASSWORD_SIZE - 1)) enteredPassword[pos++] = NULL_TERM;  // clear out the memory used for the entered master password
  return true;
}                                                                               // and check it against the same word that's stored hashed
                                                                                // in eeprom.  This word is written (hashed) to eeprom the 
                                                                                // first time ever a master password is entered.

void sha256Hash(char *password) {
  for (int i = 0; i < SHA_ITERATIONS; i++) {                                    // only seems to work correctly when SHA_INCREMENTS == 1.
    sha256HashOnce(password);
  }
}

void sha256HashOnce(char *password) {
  size_t passwordSize = strlen(password);                                       // strlen(password) == 28
  size_t posn, len;
  uint8_t value[HASHED_MASTER_PASSWORD_SZ];                                     // HASHED_MASTER_PASSWORD_SZ == 32

  sha256.reset();
  for (posn = 0; posn < passwordSize; posn += 1) {                              // posn=0|1|2|3|...
      len = passwordSize - posn;                                                // 28|27|26|25|...
      if (len > passwordSize)
          len = passwordSize;
      sha256.update(password + posn, len);                                      // password[0], 28|password[1],27|password[2],26
  }
  sha256.finalize(value, sizeof(value));
  sha256.clear();
  memcpy(password, value, HASHED_MASTER_PASSWORD_SZ);
}

void encrypt32Bytes(uint8_t *outBuffer, uint8_t *inBuffer) {
  uint8_t leftInBuffer[16];
  uint8_t rightInBuffer[16];

  memcpy(leftInBuffer, inBuffer, 16);
  memcpy(rightInBuffer, inBuffer + 16, 16);
  
  aes.encryptBlock(leftInBuffer, leftInBuffer);
  aes.encryptBlock(rightInBuffer, rightInBuffer);
  
  memcpy(outBuffer, leftInBuffer, 16);
  memcpy(outBuffer + 16, rightInBuffer, 16);
}

void decrypt32(uint8_t *outBuffer, uint8_t *inBuffer) {                         // Necessary because blocksize of AES128/256 = 16 bytes.
  uint8_t leftInBuf[16];
  uint8_t rightInBuf[16];

  memcpy(leftInBuf, inBuffer, 16);
  memcpy(rightInBuf, inBuffer + 16, 16);
  
  aes.decryptBlock(leftInBuf, leftInBuf);                                       // decrypt the buffer 
  aes.decryptBlock(rightInBuf, rightInBuf);                                     // decrypt the buffer 

  memcpy(outBuffer, leftInBuf, 16);
  memcpy(outBuffer + 16, rightInBuf, 16);
}

//- EEPROM functions

void writeAllToEEProm(uint8_t *accountName, 
                      uint8_t *username, 
                      uint8_t *password, 
                      uint8_t pos)        {                                     // used by delete account and factory reset.
  eeprom_write_bytes(GET_ADDR_ACCT(pos), accountName, ACCOUNT_SIZE);
  eeprom_write_bytes(GET_ADDR_USER(pos), username, USERNAME_SIZE);
  eeprom_write_bytes(GET_ADDR_PASS(pos), password, PASSWORD_SIZE);
}

void countAccounts() {                                                          // count all of the account names from EEprom.
  acctCount = 0;
  tailPosition = headPosition;
  uint8_t nextPos = getNextPtr(headPosition);
  if (nextPos == headPosition)   {                                              // defense against having head pointing to itself
    eeprom_write_bytes(GET_ADDR_NEXT_POS(headPosition),
                       INITIAL_MEMORY_STATE_BYTE,
                       NEXT_POS_SIZE                   );
    nextPos = getNextPtr(headPosition);
  }
  while(nextPos != INITIAL_MEMORY_STATE_BYTE) {
    acctCount++;
    tailPosition = nextPos;
    nextPos = getNextPtr(nextPos);
    if(tailPosition == nextPos) {                                               // more defense
      eeprom_write_bytes(GET_ADDR_NEXT_POS(tailPosition),
                         INITIAL_MEMORY_STATE_BYTE,
                         NEXT_POS_SIZE                   );
      nextPos = getNextPtr(tailPosition);
    }
  }
}

uint8_t getNextFreeAcctPos() {                                                  // return the position of the next EEprom location for account name marked empty.
  for(uint8_t acctPos = 0; acctPos <= CREDS_ACCOMIDATED; acctPos++) {
      if (read_eeprom_byte(GET_ADDR_ACCT(acctPos)) == 
          INITIAL_MEMORY_STATE_BYTE                     ) {
        return acctPos;
      }
  }
  return INITIAL_MEMORY_STATE_BYTE;
}

void readAcctFromEEProm(uint8_t pos, uint8_t *buf) {
  if (pos > -1) {
    read_eeprom_array(GET_ADDR_ACCT(pos), buf, ACCOUNT_SIZE, true);
  } else {
    buf[0] = NULL_TERM;
  }
  if (buf[0] == INITIAL_MEMORY_STATE_BYTE) {
    buf[0] = NULL_TERM;                                                         // 8 bit twos complement of 255 or 0xFF
  } else {
    setKey(pos);
    decrypt32(buf, buf);
  }
}

void readUserFromEEProm(uint8_t pos, uint8_t *buf) {
  if (pos > -1) {
    read_eeprom_array(GET_ADDR_USER(pos), buf, USERNAME_SIZE, true);
  } else {
    buf[0] = NULL_TERM;
  }
  if (buf[0] == INITIAL_MEMORY_STATE_BYTE) {
    buf[0] = NULL_TERM;
  } else {
    setKey(pos);
    decrypt32(buf, buf);
  }
}

void readPassFromEEProm(uint8_t pos, uint8_t *buf) {                            // TODO: reduce readPassFromEEProm, readUserFromEEProm and readAcctFromEEProm to a single function.
  if (pos > -1) {
    read_eeprom_array(GET_ADDR_PASS(pos), buf, PASSWORD_SIZE, true);
  } else {
    buf[0] = NULL_TERM;
  }
  if (buf[0] == INITIAL_MEMORY_STATE_BYTE) {
    buf[0] = NULL_TERM;
  } else {
    setKey(pos);
    decrypt32(buf, buf);
  }
}

void readStyleFromEEProm(uint8_t pos, char *buf) {
  if (pos > -1) {
    read_eeprom_array(GET_ADDR_STYLE(pos), buf, STYLE_SIZE, true);
  } else {
    buf[0] = NULL_TERM;
  }
  if (buf[0] == INITIAL_MEMORY_STATE_CHAR) buf[0] = NULL_TERM;
}

void readCredSaltFromEEProm(uint8_t pos, uint8_t *buf) {
  if (pos > -1) {
    read_eeprom_array(GET_ADDR_CRED_SALT(pos), buf, CRED_SALT_SIZE, true);
  } else {
    buf[0] = NULL_TERM;
  }
  if (buf[0] == INITIAL_MEMORY_STATE_CHAR) buf[0] = NULL_TERM;
}

uint8_t getListHeadPosition() {                                                 // returns the position of the first element in the linked list
  uint8_t listHead = read_eeprom_byte(GET_ADDR_LIST_HEAD);
  if (listHead == INITIAL_MEMORY_STATE_BYTE) { 
    listHead = getNextFreeAcctPos();
    headPosition = listHead;
    writeListHeadPos();
  }
  return listHead;
}

uint8_t getNextPtr(uint8_t pos) {                                               // given position, returns the address of the next element in the linked list
  return read_eeprom_byte(GET_ADDR_NEXT_POS(pos));
}

uint8_t getPrevPtr(uint8_t pos) {                                               // given position, returns the position of the previous element in the linked list
  return read_eeprom_byte(GET_ADDR_PREV_POS(pos));
}

void writeNextPtr(uint8_t pos, uint8_t nextPtr) {                               // writes the next pointer to EEprom for position, pos.
  write_eeprom_byte(GET_ADDR_NEXT_POS(pos), nextPtr);
}

void writePrevPtr(uint8_t pos, uint8_t prevPtr) {                               // writes the previous pointer to EEprom for position, pos.
  write_eeprom_byte(GET_ADDR_PREV_POS(pos), prevPtr);
}

void writeLoginFailures() {                                                     // writes the number of login failures to EEprom
  write_eeprom_byte(GET_ADDR_LOGIN_FAILURES, loginFailures);
}

void writeResetFlag(uint8_t buf) {                                              // writes the value of the reset flag to EEprom
  write_eeprom_byte(GET_ADDR_RESET_FLAG, buf);
}

void writeShowPasswordsFlag() {
  write_eeprom_byte(GET_ADDR_SHOW_PW, showPasswordsFlag);
}

void writeListHeadPos() {                                                       // writes the position of the beginning of the linked list to EEprom
  write_eeprom_byte(GET_ADDR_LIST_HEAD, headPosition);
}
                                                                                // This function is used by the other, higher-level functions
                                                                                // to prevent bugs and runtime errors due to invalid addresses.
boolean eeprom_is_addr_ok(unsigned int addr) {                                  // Returns true if the address is between the
  return ((addr >= MIN_AVAIL_ADDR) && (addr <= MAX_AVAIL_ADDR));                // minimum and maximum allowed values, false otherwise.
}
                                                                                // Writes a sequence of bytes to eeprom starting at the specified address.
                                                                                // Returns true if the whole array is successfully written.
                                                                                // Returns false if the start or end addresses aren't between
                                                                                // the minimum and maximum allowed values.
                                                                                // When returning false, nothing gets written to eeprom.
boolean eeprom_write_bytes( uint16_t startAddr,                                 // TODO: cut out a lot of the boundry checking to reduce the size of this function
                            const uint8_t* buf,
                            uint8_t numBytes) {
                                                                                // counter
  uint8_t i;
                                                                                // both first byte and last byte addresses must fall within
                                                                                // the allowed range 
  if (!eeprom_is_addr_ok(startAddr) || 
      !eeprom_is_addr_ok(startAddr + numBytes)) {
    return false;
  }
  if (numBytes > EEPROM_BYTES_PER_PAGE) numBytes = EEPROM_BYTES_PER_PAGE;
  write_eeprom_array(startAddr, buf, numBytes);
  return true;
}

boolean eeprom_write_int_bytes( unsigned int startAddr,                         // given a start address, a buffer and a byte count,
                                const uint8_t* buf,                             // writes the buffer to EEprom
                                uint8_t numBytes) {
                                                                                // both first byte and last byte addresses must fall within
                                                                                // the allowed range 
  if ((startAddr > MAX_AVAIL_INT_ADDR) ||
      (startAddr < MIN_AVAIL_INT_ADDR) ||
      ((startAddr + numBytes) > MAX_AVAIL_INT_ADDR)) {
    return false;
  }
  for (uint8_t i = 0; i < numBytes; i++) {                                      // iterate over every byte in the buffer
    EEPROM.write(startAddr + i, buf[i]);                                        // write out each byte
  }
  return true;                                                                  // read was successful
}

void eeprom_read_int_string(    uint16_t addr,                                  // reads a string from the internal EEprom starting from the specified address
                                unsigned char* buffer, 
                                uint8_t bufSize) {
  uint8_t bytesRead;                                                            // number of bytes read so far
  bytesRead = 0;                                                                // initialize byte counter
  buffer[bytesRead] = EEPROM.read(addr + bytesRead++);                          // store it into the user buffer
  while ( (bytesRead < bufSize) && ((addr + bytesRead) <= MAX_AVAIL_ADDR) ) {   // eliminate check for NULL_TERM because of hashing
                                                                                // if no stop condition is met, read the next byte from eeprom
    buffer[bytesRead] = EEPROM.read(addr + bytesRead++);                        // store it into the user buffer
  }
}

void InitializeEEProm(void) {                                                   // Initializes all of external EEprom; sets every address = 255.
  boolean colorRed = true;                                                      // show purple during healthy EEprom initialize
  uint16_t pageAddress = MIN_AVAIL_ADDR;
  while (pageAddress <= MAX_AVAIL_ADDR) {
    if (pageAddress%256==0) {
      if (colorRed) {
        setRed();
        colorRed = false;
      } else {
        setBlue();
        colorRed = true;
      }
    }
    EEPROM_writeEnable();
    SLAVE_PRIMARY_SELECT;
    SPI_tradeByte(EEPROM_WRITE);
    EEPROM_send16BitAddress(pageAddress);
    for (uint8_t i = 0; i < EEPROM_BYTES_PER_PAGE; i++) {
      SPI_tradeByte(INITIAL_MEMORY_STATE_BYTE);
    }
    SLAVE_PRIMARY_DESELECT;
    pageAddress += EEPROM_BYTES_PER_PAGE;
    while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {;
    }
  }
  setBlue();
}

void InitializeIntEEProm() {                                                    // Initializes all of internal EEprom; sets every address = 255.
  boolean colorRed = true;
  for (uint16_t addr = MIN_AVAIL_INT_ADDR; addr <= MAX_AVAIL_INT_ADDR; addr++) {
    if(addr%64==0) {                                                            // RGB is purple when initializing EEprom
      if (colorRed) {
        setRed();
        colorRed = false;
      } else {
        setBlue();
        colorRed = true;
      }
    }
    EEPROM.write(addr, INITIAL_MEMORY_STATE_BYTE);                              // second parameter is a byte.  TODO: do a bulk write to improve speed
  }
  setBlue();
}

//- SPI logic                                                                      from "Make: AVR Programming", Chapter 16. SPI, by Elloit Williams, Published by Maker Media, Inc, 2014
                                                                                // https://www.safaribooksonline.com/library/view/make-avr-programming/9781449356484/ch16.html
void initSPI(void) {
  SPI_SS_PRIMARY_DDR |= (1 << SPI_SS_PRIMARY);                                  // set SS output for primary EEprom chip
  SPI_SS_PRIMARY_DDR |= (1 << SPI_SS_PRIMARY);                                  // set SS output for primary EEprom chip
  SPI_SS_PRIMARY_PORT |= (1 << SPI_SS_PRIMARY);                                 // start off not selected (high)

  SPI_SS_SECONDARY_DDR |= (1 << SPI_SS_SECONDARY);                              // set SS output for backup EEprom chip
  SPI_SS_SECONDARY_DDR |= (1 << SPI_SS_SECONDARY);                              // set SS output for backup EEprom chip
  SPI_SS_SECONDARY_PORT |= (1 << SPI_SS_SECONDARY);                             // start off not selected (high)

  SPI_MOSI_DDR |= (1 << SPI_MOSI);                                              // output on MOSI
  SPI_MISO_PORT |= (1 << SPI_MISO);                                             // pullup on MISO
  SPI_SCK_DDR |= (1 << SPI_SCK);                                                // output on SCK

                                                                                // Don't have to set phase, polarity b/c default works with 25LCxxx chips
//  SPCR |= (1 << SPR1);                                                        // original coment said this was "div 16, safer for breadboards", but it looks like div 64
  SPCR |= (1 << SPR0);                                                          // div 16 (if alone)
  SPCR |= (1 << SPR1);                                                          // div 128 (with line above)
  //SPCR |= (1 << SPI2X);                                                       // add this to double the rate to div 8, pg. 183 of https://www.pjrc.com/teensy/atmega32u4.pdf
  SPCR |= (1 << MSTR);                                                          // clockmaster
  SPCR |= (1 << SPE);                                                           // enable
}

void SPI_tradeByte(uint8_t byte) {
  SPDR = byte;                                                                  // SPI starts sending immediately
  loop_until_bit_is_set(SPSR, SPIF);                                            // wait until done
                                                                                // SPDR now contains the received byte
}

void EEPROM_send16BitAddress(uint16_t address) {
  SPI_tradeByte((uint8_t) (address >> 8));                                      // most significant byte
  SPI_tradeByte((uint8_t) address);                                             // least significant byte
}

uint8_t EEPROM_readStatus(void) {
  SLAVE_PRIMARY_SELECT;
  SPI_tradeByte(EEPROM_RDSR);
  SPI_tradeByte(0);                                                             // clock out eight bits
  SLAVE_PRIMARY_DESELECT;
  return (SPDR);                                                                // return the result
}

uint8_t EEPROM_readStatusSecondary(void) {
  SLAVE_SECONDARY_SELECT;
  SPI_tradeByte(EEPROM_RDSR);
  SPI_tradeByte(0);                                                             // clock out eight bits
  SLAVE_SECONDARY_DESELECT;
  return (SPDR);                                                                // return the result
}

void EEPROM_writeEnable(void) {
  SLAVE_PRIMARY_SELECT;
  SPI_tradeByte(EEPROM_WREN);
  SLAVE_PRIMARY_DESELECT;
}

uint8_t read_eeprom_byte(uint16_t address) {
  SLAVE_PRIMARY_SELECT;
  SPI_tradeByte(EEPROM_READ);
  EEPROM_send16BitAddress(address);
  SPI_tradeByte(0);
  SLAVE_PRIMARY_DESELECT;
  return (SPDR);
}

void read_eeprom_array( uint16_t address, 
                        uint8_t *buffer, 
                        uint8_t sizeOfBuffer,
                        uint8_t primaryFlag   ) {                               // READ EEPROM bytes
  if (primaryFlag) {
    SLAVE_PRIMARY_SELECT;
  } else {
    SLAVE_SECONDARY_SELECT;
  }
  SPI_tradeByte(EEPROM_READ);
  EEPROM_send16BitAddress(address);
  for (uint8_t i = 0; i < sizeOfBuffer; i++) {
    SPI_tradeByte(0);
    *buffer++ = SPDR;                                                           // get data byte
  }
  if (primaryFlag) {
    SLAVE_PRIMARY_DESELECT;
  } else {
    SLAVE_SECONDARY_DESELECT;
  }
}

void write_eeprom_byte(uint16_t address, uint8_t byte) {
  EEPROM_writeEnable();
  SLAVE_PRIMARY_SELECT;
  SPI_tradeByte(EEPROM_WRITE);
  EEPROM_send16BitAddress(address);
  SPI_tradeByte(byte);
  SLAVE_PRIMARY_DESELECT;
  while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {;
  }
}

void write_eeprom_array(uint16_t address, 
                        uint8_t *buffer, 
                        uint8_t sizeOfBuffer) {
  EEPROM_writeEnable();
  SLAVE_PRIMARY_SELECT;
  SPI_tradeByte(EEPROM_WRITE);
  EEPROM_send16BitAddress(address);

  for (uint8_t i=0;i<sizeOfBuffer;i++)
  {
    SPI_tradeByte(buffer[i]);
  }
  SLAVE_PRIMARY_DESELECT;
  while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {;
  }
}

void CopyChip(uint8_t restoreFlag) {                                            // Make a byte for byte duplicate of the backup external EEprom device
  DisplayLine2("Copying...");
  uint8_t buffer[EEPROM_BYTES_PER_PAGE];                                        // make a buffer the same size as the page size.
  for ( uint16_t address = MIN_AVAIL_ADDR; 
        address < MAX_AVAIL_ADDR; 
        address += EEPROM_BYTES_PER_PAGE) {
    setYellow();
    read_eeprom_array(address, buffer, EEPROM_BYTES_PER_PAGE, !restoreFlag);    // if not restoring we are backing up, read from primary chip
    if(!restoreFlag) {                                                          // if we're backing up
      SLAVE_SECONDARY_SELECT;                                                   // select the secondary/backup chip
    } else {                                                                    // otherwise we are restoring
      SLAVE_PRIMARY_SELECT;                                                     // select the primary chip
    }
    SPI_tradeByte(EEPROM_WREN);                                                 // write enable primary
    if(!restoreFlag) {
      SLAVE_SECONDARY_DESELECT;
    } else {
      SLAVE_PRIMARY_DESELECT;
    }
    if(!restoreFlag) {
      SLAVE_SECONDARY_SELECT;
    } else {
      SLAVE_PRIMARY_SELECT;
    }
    SPI_tradeByte(EEPROM_WRITE);
    EEPROM_send16BitAddress(address);
    for (uint8_t i=0;i<EEPROM_BYTES_PER_PAGE;i++) {                             // write the page out byte for byte to the primary
      SPI_tradeByte(buffer[i]);
    }
    if(!restoreFlag) {
      SLAVE_SECONDARY_DESELECT;
    } else {
      SLAVE_PRIMARY_DESELECT;
    }
    if (!restoreFlag) {
      while (EEPROM_readStatusSecondary() & _BV(EEPROM_WRITE_IN_PROGRESS));
    } else {
      while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS));
    }
  }
  headPosition = getListHeadPosition();                                         // read the head of the doubly linked list that sorts by account name
  acctPosition = headPosition;
  countAccounts();                                                              // count the number of populated accounts in EEprom
  setGreen();
  DisplayLine2("Copied");
}

void writePointers(uint8_t accountPosition, uint8_t *accountName) {             // traverse through the linked list finding the right spot to insert this record in the list
//Serial.print(F("--------------"));
//Serial.print(F("1: "));Serial.println(accountPosition);
  if ((headPosition    == 0) &&
      (tailPosition    == 0) &&
      (accountPosition == 0)   ) {                                              // this is the first element added to the linked list
//  Serial.println(F("2"));
    writePrevPtr(accountPosition, INITIAL_MEMORY_STATE_BYTE);
    writeNextPtr(accountPosition, INITIAL_MEMORY_STATE_BYTE);
    writeListHeadPos();
    return;
  }
  
  uint8_t acctBuf[ACCOUNT_SIZE];                                                // a buffer large enough to accomodate the account name
  uint8_t currentPosition = headPosition;                                       // pointer to the position we're at as we step through the linked list
//Serial.print(F("3: "));Serial.println(currentPosition);
  uint8_t prevPosition = getPrevPtr(currentPosition);                           // should always be INTIAL_MEMORY_STATE_BYTE.  This IS necessary.
//Serial.print(F("4: "));Serial.println(prevPosition);
  readAcctFromEEProm(headPosition, acctBuf);                                    // reading the accountName for the head
//Serial.print(F("5: "));Serial.println((char *)acctBuf);
  while ((currentPosition != INITIAL_MEMORY_STATE_BYTE   ) && 
         (strncmp(acctBuf, accountName, ACCOUNT_SIZE) < 0)     ) {              // if Return value < 0 then it indicates str1 is less than str2.
    prevPosition = currentPosition;                                             // save prevPosition as currentPosition because we'll eventually step over the element that's > accountPosition
//  Serial.print(F("6: "));Serial.println(prevPosition);
    currentPosition = getNextPtr(currentPosition);                              // move to the next element in the linked list
//  Serial.print(F("7: "));Serial.println(currentPosition);
    readAcctFromEEProm(currentPosition,acctBuf);                                // read that account name from EEprom
//  Serial.print(F("8: "));Serial.println((char *)acctBuf);
  }
  if(currentPosition == headPosition) {                                         // inserting before the first element in the list
    headPosition = accountPosition;
//  Serial.print(F("9: "));Serial.println(headPosition);
    writeListHeadPos();
  }
  if (currentPosition == INITIAL_MEMORY_STATE_BYTE) {                           // inserting an element at the end of the linked list
    tailPosition = accountPosition;
//  Serial.print(F("10: "));Serial.println(tailPosition);
  }
  writePrevPtr(accountPosition, prevPosition   );                               // insert between prevPosition and currentPosition
//Serial.print(F("11: "));Serial.println(prevPosition);
  writeNextPtr(accountPosition, currentPosition);
//Serial.print(F("12: "));Serial.println(currentPosition);
  if (prevPosition != INITIAL_MEMORY_STATE_BYTE) {                              // if we're not the new head
//  Serial.print(F("13"));
    writeNextPtr(prevPosition, accountPosition);                                // update the next pointer of the previous element with the account position.
  }
  if (currentPosition != INITIAL_MEMORY_STATE_BYTE) {                           // if we're not the next element of the tail
//  Serial.print(F("14: "));Serial.println(accountPosition);
    writePrevPtr(currentPosition, accountPosition);                             // write set the previous pointer of the current element to the account position
  }
}
/*
void FixCorruptLinkedList() {                                                   // Rebuild the linked list to fix any issues with the pointers
//  DisableInterrupts();
  setRed();
  DisplayLine2("Fixing corrupt");
  headPosition = 0;
  tailPosition = 0;
  for (uint8_t pos = 0; pos <= CREDS_ACCOMIDATED; pos++) {                      // Visit every possible location for a set of creds
    uint8_t buffer[ACCOUNT_SIZE];                                               // a buffer that will accomodate the account name
    buffer[0] = INITIAL_MEMORY_STATE_BYTE;
    readAcctFromEEProm(pos, buffer);                                            // get the name of the account at this position, if any
    if (buffer[0] != INITIAL_MEMORY_STATE_BYTE) {                               // if true then creds have been written to this location
      writePointers(pos, buffer);                                               // set the previous and next pointers on this set of credentials
    }
  }
  writeListHeadPos();
  headPosition = getListHeadPosition();                                         // read the head of the doubly linked list that sorts by account name
  acctPosition = headPosition;
  tailPosition = findTailPosition();                                            // find the tail of the doubly linked list that sorts by account name
  position = 0;
  countAccounts();                                                              // count the number of populated accounts in EEprom
  setGreen();
  DisplayLine2("Fixed corrupt");
//  EnableInterrupts();
}
*/
