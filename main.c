#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#include "fonts.h"


/*Number of rows in display buffer*/
#define BUFF_ROW                            8
/*Number of columns in display buffer.(Consider your display width while changing this.)*/
#define BUFF_COL                            17


/*String to statically print or scroll(Change it to what needs to be displayed)*/
static char string0_to_print[] = "Hello World!";
/*String to statically print or scroll(Change it to what needs to be displayed)*/
static char string1_to_print[] = "Hello Friends!";
/*String to statically print or scroll(Change it to what needs to be displayed)*/
static char string2_to_print[] = "This is my first post on LinkedIn!";
/*Character in the form the text needs to be displayed*/
static char pattern0_char = '*';
/*Character in the form the text needs to be displayed*/
static char pattern1_char = '#';
/*Character in the form the text needs to be displayed*/
static char pattern2_char = 'o';
/*Sets the speed of scrolling text, change this to get desired speed.*/
static uint8_t scrolling_speed1 = 2;
/*Sets the speed of scrolling text, change this to get desired speed.*/
static uint8_t scrolling_speed2 = 1;
/*Variable to hold cursor position, which is set to origin i.e (0,0)*/
static COORD home = {0, 0};
/*Display Buffer to hold the text data*/
static uint8_t display_buffer[BUFF_ROW][BUFF_COL];


/*Holds the current index of string array*/
static uint16_t curr_str_idx = 0;
/*Holds the current bit injected into display buffer*/
static uint8_t curr_font_bit_pos = 0;
/*Flag to check if all characters from the string are injected into display buffer.*/
static bool str_complete = false;
/*Counter to shift the whole display till the last string is completely gone.*/
static int16_t shift_counter = BUFF_COL * 8;


/*Copies the MSB of font buffer to LSB of nth column of display buffer.
This is done with all the characters in the string. This is like feeding
font data into one end of the display buffer. Injecting of bits into
display buffer.*/
bool copyAlphToBuff(char *str, uint16_t str_len)
{
    bool status = false;
    /*Add alphabets only if characters in string are remaining to display*/
    if( !str_complete )
    {
        /*Clears the LSB bit of nth column in display buffer, just a precaution.*/
        for (uint8_t row = 0; row < BUFF_ROW; row++)
        {
            display_buffer[row][BUFF_COL - 1] &= ~0x01;
        }

        /*The MSB of font data is copied to LSB of nth column of display buffer*/
        for(uint8_t row = 0; row < BUFF_ROW; row++)
        {
            char temp_ch = font8x8[(uint8_t)str[curr_str_idx]][row];

            display_buffer[row][BUFF_COL - 1] |= ((temp_ch >> (curr_font_bit_pos)) & 0x01);
        }

        /*Bit position incremented*/
        curr_font_bit_pos++;

        /*Check if the bit position is greater than 7.*/
        if(curr_font_bit_pos > 7)
        {
            /*Reset bit position to zero*/
            curr_font_bit_pos = 0;
            /*Move to next character in the string*/
            curr_str_idx++;
        }
    }

    /*Handle the condition if all the characters from the string are injected*/
    if(curr_str_idx >= str_len)
    {
        /*Set the string complete flag*/
        str_complete = true;
        /*Decrement the shift counter*/
        shift_counter--;

        /*Counter elapsed?*/
        if(shift_counter == 0)
        {
            /*Reset string index to zero*/
            curr_str_idx = 0;
            /*Reset the bit position*/
            curr_font_bit_pos = 0;
            /*Reset string complete flag*/
            str_complete = false;
            /*Reinitialize the shift counter*/
            shift_counter = BUFF_COL * 8;
            /*Return status that scroll is officially completed*/
            status = true;
        }
    }

    return status;
}


/*Copies the font data column by column to display buffer to imitate static text display.*/
void copyStatTextToBuff(char *str, uint16_t str_len)
{
    /*Copies font data to display buffer*/
    for(uint8_t col = 0; col < str_len; col++)
    {
        for(uint8_t row = 0; row < BUFF_ROW; row++)
        {
            uint8_t temp = font8x8[(uint8_t)str[col]][row];
            uint8_t reversed;

            /*Reverse font bits to avoid mirror image*/
            for (int8_t bit = 0; bit < 8; bit++)
            {
                reversed <<= 1;
                reversed |= (temp >> bit) & 0x01;
            }

            display_buffer[row][col] = reversed;
        }
    }
}

/*Prints latest updated display buffer to the console using the pattern character provided by the user.*/
void printBuff(char pat)
{
    /*In the display buffer, in place of 1 in the bit position pattern character is printed and a space is
    printed in place of 0.*/
    for(uint8_t row = 0; row < BUFF_ROW; row++)
    {
        for(uint8_t col = 0; col < BUFF_COL; col++)
        {
            uint8_t temp = display_buffer[row][col];

            for(int8_t shift = 7; shift > 0; shift--)
            {
                if((temp >> shift) & 0x01)
                {
                    printf("%c", pat);
                }
                else
                {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }
}

/*Shifts the display by 1 it to the left to achieve scrolling effect.*/
void shiftDisplay()
{
    /*The display is shifted to left by 1 bit by copying the column MSB to previous column LSB
    in display buffer. This creates a left scrolling effect.*/
    for(uint8_t col = 0; col < BUFF_COL; col++)
    {
        for(uint8_t row = 0; row < BUFF_ROW; row++)
        {
            if(col != 0)
            {
                display_buffer[row][col - 1] |= ((display_buffer[row][col] >> 7) & 0x01);
            }

            display_buffer[row][col] <<= 1;
        }
    }
}

/*Function to scroll text from right to left.*/
void scrollText(char *str, uint16_t str_len, char pat, uint8_t speed)
{
    /*Holds the current scroll status*/
    bool scroll_complete = false;

    do
    {
        /*Adds font data to display buffer*/
        scroll_complete = copyAlphToBuff(str, str_len);
        /*Prints the display buffer to the console*/
        printBuff(pat);
        /*Sleeps for few milliseconds, change this timing to adjust scrolling speed*/
        Sleep(10 * speed);
        /*Sets the cursor position to origin (0,0)*/
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);
        /*Shifts the display to left*/
        shiftDisplay();

    }
    while(!scroll_complete);

    /*Set cursor to home position*/
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);
    /*Clear display buffer*/
    memset(display_buffer, 0, BUFF_COL * BUFF_ROW);
}

/*Prints static text on console*/
void staticText(char *str, uint16_t str_len, char pat)
{
    /*Copies text to display buffer*/
    copyStatTextToBuff(str, str_len);
    /*Prints the display buffer on console.*/
    printBuff(pat);
    /*Set cursor to home position*/
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);
    /*Clear display buffer*/
    memset(display_buffer, 0, BUFF_COL * BUFF_ROW);
}

int main()
{
    while(1)
    {
        /*Display static text*/
        staticText(string0_to_print, strlen(string0_to_print), pattern0_char);
        /*Just a delay*/
        Sleep(5000);
        /*Scroll text*/
        scrollText(string1_to_print, strlen(string1_to_print), pattern1_char, scrolling_speed1);
        scrollText(string2_to_print, strlen(string2_to_print), pattern2_char, scrolling_speed2);

    }
    return 0;
}
