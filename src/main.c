/*
 * main.c
 *
 *  Created on: Jun 16, 2015
 *      Author: David Etler
 */

#define _N_STEPS 365*5
#define _DT 60*60*24
#define _DELIMITER ','

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "planet.h"
#include "universe.h"

const double AU = 1.496e11; //one astronomical unit

int parse_universe(universe* universe, char* file_name);

int main(int argc, char* argv[])
{
    universe* my_universe = new_universe();

    if (argc == 1)
    {
        //default system

        planet* my_planet;

        my_planet = new_planet("Earth", (double[3] ) { 1.52e11, 0.0, 0.0 }, (double[3] ) { 0.0, 29316, 0.0 }, 5.972e24);
        add_planet(my_universe, my_planet);

        my_planet = new_planet("Sun", (double[3] ) { 0.0, 0.0, 0.0 }, (double[3] ) { 0.0, 0.0, 0.0 }, 1.988435e30);
        add_planet(my_universe, my_planet);
    }
    else
    {
        //load saved system
        int p = parse_universe(my_universe, argv[1]);
        if(p != 0)
        {
            //catch errors
            if(p == -1)
            {
                printf("Error: planet file malformed, aborting.\n");
                free_universe(my_universe);
                return 1;
            }
            if(p == -2)
            {
                printf("Error: planet file not found, aborting.\n");
            }
        }
    }

    for (int n = 0; n < _N_STEPS; n++)
    {
        //don't clutter the output with 1000s of lines of information
        //make number smaller for more verbose output
        if (n % 1 == 0)
        {
            //print info for all planets (csv)
            for (int i = 0; i < my_universe->planet_list->length; i++)
            {
                if (i != 0)
                {
                    printf("%c", _DELIMITER);
                }
                printf("%s", my_universe->planet_list->list_address[i]->name);
                for (int j = 0; j < 3; j++)
                {
                    printf("%c%f", _DELIMITER, my_universe->planet_list->list_address[i]->pos[j] / AU);
                }
            }
            printf("\n");
        }
        universe_update_midpoint(my_universe);
    }

    free_universe(my_universe);

    return 0;
}

/*
 * Parses the universe from a file containing the initial state
 *
 * Lightweight CSV parser, character-by-character. Assumes the input is valid.
 * Will probably crash or cause undefined behavior if input is invalid.
 */
int parse_universe(universe* universe, char* fname)
{
    char const* const file_name = fname;
    FILE* file = fopen(file_name, "r"); //Open the file for reading
    char line[256]; //buffer to store lines

    int body_num = 0;
    //while there's a new line
    if (file != NULL)
    {
        //to store state variables
        char* name;
        double pos[3] = { 0, 0, 0 };
        double vel[3] = { 0, 0, 0 };
        double mass = 0;

        while (fgets(line, sizeof(line), file))
        {
            if (feof(file) || strlen(line) == 0)
            {
                //finished reading file, escape
                break;
            }

            int item_num = 0;
            int item_start = 0; //starting index of current item

            //scan the line for all 8 items of information (Name, pos_1, pos_2, pos_3, vel_1, vel_2, vel_3, mass)
            while (item_num < 8)
            {
                int curr_char = item_start;

                //find next comma/line end
                while (line[curr_char] != ',' && line[curr_char] != '\n')
                {
                    curr_char++;
                }

                int item_end = curr_char;
                char* item = malloc((item_end - item_start + 1) * sizeof(char)); //allocate space for current item
                curr_char = item_start;

                //copy the current item value to item
                while (curr_char < item_end)
                {
                    item[curr_char - item_start] = line[curr_char];
                    curr_char++;
                }
                item[item_end - item_start] = '\0'; //null terminator

                //we now have the item in a string called `item` and need to use it
                if (item_num == 0)
                {
                    //the name
                    name = strdup(item);
                }
                if (item_num > 0 && item_num < 4)
                {
                    //the position
                    pos[item_num - 1] = strtod(item, 0);
                }
                if (item_num > 3 && item_num < 7)
                {
                    //the velocity
                    vel[item_num - 4] = strtod(item, 0);
                }
                if (item_num == 7)
                {
                    //the mass
                    mass = strtod(item, 0);
                }

                //move onto next item

                //if we're already at the end of the line, and we haven't found enough items
                if(line[curr_char] == '\n' && item_num < 7)
                {
                    free(item);
                    fclose(file);
                    return -1;
                }
                item_num++;
                item_start = item_end + 1; //next item starts right after this one ends
                free(item);

            }
            //create a new planet with the value from the file, and add to universe
            planet* my_planet;
            my_planet = new_planet(name, pos, vel, mass);
            add_planet(universe, my_planet);
            body_num++;
            free(name);
        }
        //close the file
        fclose(file);
    }
    else
    {
        return -2;
    }
    return 0;

}
