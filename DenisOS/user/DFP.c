#include "DFP.h"

#define noOfPhilosophers 16

int is_thought_prime( uint32_t x ) {
    if ( !( x & 1 ) || ( x < 2 ) ) {
        return ( x == 2 );
    }
    
    for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
        if( !( x % d ) ) {
            return 0;
        }
    }
    
    return 1;
}

void think(){
    for( int i = 0; i < 25; i++ ) {
        write( STDOUT_FILENO, "Think", 5 );
        
        uint32_t lo = 1 <<  8;
        uint32_t hi = 1 << 16;
        
        for( uint32_t x = lo; x < hi; x++ ) {
            int r = is_thought_prime( x );
        }
    }
    write( STDOUT_FILENO, "Stopped Thinking\n", 17 );
}

void eat(){
    for( int i = 0; i < 25; i++ ) {
        write( STDOUT_FILENO, "Eating", 6 );        
        for( uint32_t x = (uint32_t) (1<<8); x < (uint32_t) (1<<16); x++ ) {
            int r = is_thought_prime( x );
        }
    }
    write( STDOUT_FILENO, "Stopped Eating\n", 15 );
}

int max(int a, int b){
    if (a>=b) return a;
    return b;
}
int min(int a, int b){
    if (a<=b) return a;
    return b;
}

void main_DFP(){
    int index = sem_open();
    for (int i = index; i<(index+noOfPhilosophers); i++){
        pid_t pid = fork();
        if (pid==0){
            while (1) {
                think();
                sem_wait(min(i, (i+1) % noOfPhilosophers));
                sem_wait(max(i, (i+1) % noOfPhilosophers));
                eat();
                sem_post((i+1) % noOfPhilosophers);
                sem_post(i);
            }
        }
    }
    write(STDOUT_FILENO, "DONE\n", 5);
    while (1){
        asm volatile("nop");
    }
}
