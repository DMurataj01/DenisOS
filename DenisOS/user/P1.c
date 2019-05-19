/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P1.h"

void main_P1() {
    while( 1 ) {
        write( STDOUT_FILENO, "P1\n", 3 );
        
        pid_t f = fork();
        if (f==0){
            while(1){
                write(STDOUT_FILENO, "FKCHILD\n", 8);
            }
        } else{
            write(STDOUT_FILENO, "PRT\n", 4);
            for( int i = 0; i < 0x10000000; i++ ) {
                asm volatile ("nop");
            }
        }
    }
    exit( EXIT_SUCCESS );
}
