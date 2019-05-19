//Denisi Murataj.

#include "hilevel.h"
#include "console.h"

#define max 25              //maximum no. of programs.
#define max_pipes 10        //maximum no. of pipes
#define max_endpoints 5     //maximum no. of endpoints [should be twice the number of pipes]
#define max_semaphores 50   //maximum no. of semaphores.


int noOfProgs = 0; //keeps track of the number of running programs -> value is set in RESET.

pcb_t pcb[ max ]; pcb_t* current = NULL;

int max_PID = 0; int max_PIPEID = 0; int max_ENDPOINTID = 2; int max_SEM = 0; //DO NOT TOUCH -> used to set unique identifiers [ 0, 0, 2, 0].

pipe_t pipes[max_pipes]; endpoint_t endpoints[max_endpoints];

sem_t semaphores[max_semaphores];

extern void     main_console();
extern void     main_DFP();
extern uint32_t tos_progs;

uint32_t stacksize = 0x00001000;

uint32_t tos_prog( int i ){
    return ((uint32_t) &tos_progs - (stacksize*i));
}
sem_t getNewSemaphore(){
    if (max_SEM < max_semaphores){
        max_SEM++;
        return max_SEM;
    }
    else {
        return -1;
    }
}
int getFreeSlot() {
    //check status.
    for (int i=0; i< max; i++){
        if (pcb[i].status == STATUS_TERMINATED){
            return i;
        }
    }
    return -1;
}
pid_t getNewPID(){
    if (max_PID < max - 1){
        max_PID++;
        return max_PID;
    }
    else {
        return -1;
    }
}
pipeid_t getNewPipeID(){
    if (max_PIPEID < max_pipes){
        max_PIPEID++;
        return max_PIPEID;
    }
    else {
        return -1;
    }
}
epid_t getNewEndpointID(){
    if (max_ENDPOINTID < max_endpoints + 2){ // 2 comes from offset of starting max_ENDPOINTID
        max_ENDPOINTID++;
        return max_ENDPOINTID;
    }
    else {
        return -1;
    }
}
int getPCBIndex(pid_t pid){
    for (int i=0; i<max; i++){
        if (pid == pcb[i].pid){
            return i;
        }
    }
    return -1;
}
int getIndex(){
    for (int i=0; i<max; i++){
        if (current->pid == pcb[i].pid){
            return i;
        }
    }
    return -1;
}
pid_t getCurrentPID(ctx_t* ctx){
    int i = getIndex();
    return pcb[i].pid;
}
void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
    char prev_pid = '?', next_pid = '?';

    if( NULL != prev ) {
        memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
        prev_pid = '0' + prev->pid;
    }
    if( NULL != next ) {
        memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
        next_pid = '0' + next->pid;
    }

    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );
    PL011_putc( UART0, '\n',      true );
    
    
    current = next;                             // update executing index to P_{next}

    return;
}
void schedule( ctx_t* ctx){
    int noOfProgramsFound = 0;
    int index_current = getIndex();
    int index = -1;
    
    int best_index = -1;
    int best_priority = -1;
    
    //increment age of each process.
    for (int i=0; i < max; i++){
        if (pcb[i].status != STATUS_TERMINATED && pcb[i].status != STATUS_WAITING && pcb[i].status != STATUS_EXECUTING){
            pcb[i].age++;
        }
    }
    if (noOfProgs == 1){
        //add check current running program is me.
        return;
    }
    //if there are running programs.
    else if (noOfProgs > 0){
        for (int i=0; i < max; i++){
            //Current program being run.
            if (current->pid == pcb[i].pid){
                //get index of current program.
                index_current = i;
                noOfProgramsFound++;
                pcb[i].age = 0;
                continue;
            }
            //Program exists here.
            if (pcb[i].status != STATUS_TERMINATED && pcb[i].status != STATUS_WAITING){
                noOfProgramsFound++;
                int total_priority = (priorityWeight * pcb[i].priority) + pcb[i].age;
                if (best_index == -1 || total_priority > best_priority){
                    best_index = i;
                    best_priority = total_priority;
                }
            }
            //Dispatch when we find all the programs [ extra check to ensure we have found the current program ].
            if (noOfProgramsFound == noOfProgs && index_current != -1){
                dispatch(ctx, &pcb[index_current], &pcb[best_index]);
                pcb[best_index].status = STATUS_EXECUTING;
                pcb[best_index].age = 0;
                if (pcb[index_current].status == STATUS_EXECUTING){
                    pcb[index_current].status = STATUS_READY;
                }
                return;
            }
        }
        if (pcb[index_current].status == STATUS_EXECUTING){
            return;
        }
    }
}

void hilevel_handler_rst( ctx_t* ctx ) {
    if (current == NULL){
        memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );     // initialise 0-th PCB = P_1
        pcb[ 0 ].pid      = 1;
        pcb[ 0 ].status   = STATUS_CREATED;
        pcb[ 0 ].priority = PRIORITY_HIGH;
        pcb[ 0 ].ctx.cpsr = 0x50;
        pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
        pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_progs );
        
        max_PID = 1;
        noOfProgs = 1;
    
        for (int i =0; i<max_pipes; i++){
            memset( &pipes[ i ], 0, sizeof( pipe_t ) );
            pipes[i].pipeid = getNewPipeID();
            pipes[i].status = EMPTY;
            pipes[i].used = NO;
        }
        for (int i=0; i<max_endpoints; i++){
            memset( &endpoints[ i ], 0, sizeof( endpoint_t ) );
            endpoints[i].eid = getNewEndpointID();
            endpoints[i].pipeid = -1;
            endpoints[i].pid = -1;
            endpoints[i].used = NO;
        }
        //Empty slots for Fork.
        for (int i=noOfProgs; i<max; i++){
            memset( &pcb[ i ], 0, sizeof( pcb_t ) );
        }
        
        UART0->IMSC       |= 0x00000010; // enable UART    (Rx) interrupt
        UART0->CR          = 0x00000301; // enable UART (Tx+Rx)

        TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
        TIMER0->Timer1Load  = 0x00010000; // select period = 2^20 ticks ~= 1 sec
        TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
        TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
        TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
        TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer
        
        GICC0->PMR          = 0x000000F0; // unmask all            interrupts
        GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
        GICC0->CTLR         = 0x00000001; // enable GIC interface
        GICD0->CTLR         = 0x00000001; // enable GIC distributor
 
        int_enable_irq();

        dispatch( ctx, NULL, &pcb[ 0 ] );
        
    }
    else {
        PL011_putc( UART0, '+', true );
        PL011_putc( UART0, '+', true );
        PL011_putc( UART0, '+', true );
    }
    return;
    
}

void hilevel_handler_irq(ctx_t* ctx) {
    uint32_t id = GICC0->IAR;
    if (id == GIC_SOURCE_TIMER0){
        schedule(ctx);
        TIMER0->Timer1IntClr = 0x01;
    }
    GICC0->EOIR = id;
    return;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
    
    switch( id ) {
        case 0x00: { // 0x00 => yield()
            schedule( ctx );
            break;
        }
        case 0x01: { // 0x01 => write( fd, x, n )
            int   fd = ( int   )( ctx->gpr[ 0 ] );
            char*  x = ( char* )( ctx->gpr[ 1 ] );
            int    n = ( int   )( ctx->gpr[ 2 ] );
            
            if (fd == STDERR_FILENO || fd == STDOUT_FILENO){
                for( int i = 0; i < n; i++ ) {
                    PL011_putc( UART0, *x++, true );
                }
                ctx->gpr[ 0 ] = n;
                return;
            }
            else {
                for (int i=0; i< max_endpoints; i++){
                    pipeid_t pipeID = endpoints[i].pipeid; //get pipe identifier of this ep.
                    if (endpoints[i].eid == fd){
                        endpoints[i].pid = current->pid;       //set program is endpoint owner.
                    
                        if (endpoints[i].type == WRITE){
                            if (pipes[pipeID].status == EMPTY){
                                endpoints[i].pid = current->pid;       //set program is endpoint owner.
                                pipes[pipeID].data = *x;
                                pipes[pipeID].status = FULL;
                                //unpause read process.
                                for (int j=0; j< max_endpoints; j++){
                                    if (endpoints[j].pipeid == pipeID && endpoints[j].type == READ){
                                        int pcbIndex = getPCBIndex(endpoints[j].pid);
                                        if ((pcbIndex != -1) && pcb[pcbIndex].status == STATUS_WAITING){
                                            pcb[pcbIndex].status = STATUS_READY;
                                        }
                                        break;
                                    }
                                }
                                ctx->gpr[ 0 ] = 1; //ctx->gpr[ 0 ] = n when buffer becomes implemented.
                            }
                            else if (pipes[pipeID].status == FULL){
                                current->status = STATUS_WAITING;
                                schedule(ctx);
                            }
                            return;
                        }
                    }
                }
            }
            ctx->gpr[ 0 ] = -1;
            break;
        }
        case 0x02: { //0x02 => read(fd, x, n)
            int fd = (int ) (ctx->gpr[0]);
            char*  x = ( char* )( ctx->gpr[ 1 ] );
            int    n = ( int   )( ctx->gpr[ 2 ] );
            if ( fd == STDERR_FILENO || fd == STDIN_FILENO){
                for( int i = 0; i < n; i++ ) {
                    x[i] = PL011_getc( UART0, true );
                }
                ctx->gpr[ 0 ] = n;
                return;
            }
            else {
                //TODO: READ FROM PIPE.
                for (int i=0; i< max_endpoints; i++){
                    if (endpoints[i].eid == fd){
                        pipeid_t pipeID = endpoints[i].pipeid; //get pipe identifier of this ep.
                        if (endpoints[i].type == READ){
                            endpoints[i].pid = current->pid;       //set program is endpoint owner.
                            if (endpoints[i].type == READ) {
                                if (pipes[pipeID].status == FULL){
                                    *x = pipes[pipeID].data;
                                    pipes[pipeID].status = EMPTY;
                      
                                    //unpause write process.
                                    for (int j=0; j< max_endpoints; j++){
                                        if (endpoints[j].pipeid == pipeID && endpoints[j].type == WRITE){
                                            int pcbIndex = getPCBIndex(endpoints[j].pid);
                                            if ((pcbIndex != -1) && pcb[pcbIndex].status == STATUS_WAITING){
                                                pcb[pcbIndex].status = STATUS_READY;
                                            }
                                            break;
                                        }
                                    }
                                } else if (pipes[pipeID].status == EMPTY){
                                    //pipe is empty. put this process to waiting queue.
                                    current->status == STATUS_WAITING;
                                    schedule(ctx);
                                    return;
                                }
                            

                                ctx->gpr[ 0 ] = 1; //ctx->gpr[ 0 ] = n when buffer becomes implemented.
                                return;
                            }
                        }
                    }
                }
            }
            ctx->gpr[ 0 ] = -1;
            break;
        }
        case 0x03: { //0x03 => fork()
            int i_child = getFreeSlot();
            int i = getIndex();
        
            if (i != -1 && i_child != -1){
                pid_t pid_child = getNewPID();
                
                pcb_t *child_program = &pcb[i_child];
                memcpy ( &child_program->ctx, ctx, sizeof(ctx_t));
                
                child_program->pid = pid_child;
                child_program->status = STATUS_CREATED;
                child_program->priority = current->priority;
                child_program->ctx.gpr[0] = 0;
                
                memcpy( (void *) tos_prog(i_child) - stacksize, (void *) tos_prog(i) - stacksize, stacksize);
                child_program->ctx.sp = tos_prog(i_child) - (tos_prog(i) - ctx->sp);
                ctx->gpr[0] = pid_child;
                noOfProgs++;
            }
            else {
                ctx->gpr[0]= -1;
            }
            break;
        }
        case 0x04: { //0x04 => exit(fd, x, n)
            int i= getIndex();
            if (i != -1){
                noOfProgs = noOfProgs - 1;
                pcb[i].status = STATUS_TERMINATED;
                schedule(ctx);
            }
            break;
        }
        case 0x05: { //0x05 => exec [start executing program at address x]
            int i = getIndex();
            void*  x = ( void* )( ctx->gpr[ 0 ] );
            ctx->pc = (uint32_t) x;
            ctx->sp = (uint32_t) tos_prog(i);
            break;
        }
        case 0x06: { //0x06 => kill( pid_t pid, int x )
            int pid = (int) ( ctx->gpr[ 0 ] );
            for (int i=0; i<max; i++){
                if (pcb[i].pid == pid){
                    pcb[i].status = STATUS_TERMINATED; //set status to terminated
                    noOfProgs--;
                    ctx->gpr[0]= 0; // successful.
                    if (pcb[i].pid == current->pid){
                        schedule(ctx);
                    }
                    return;
                }
            }
            ctx->gpr[0] = -1;
            break;
        }
        case 0x09: { //PIPE [input *fd).
            //writeShit("PIPE: ", 6);
            
            int* fd = (int*) ctx->gpr[0];
            //init file-descriptors. where i&j are the endpoints.
            int index_pipe = -1;
            int index_ep1 = -1;
            int index_ep2 = -1;
            epid_t ep1_ID = -1;
            epid_t ep2_ID = -1;
            
            
            //GET PIPE INDEX;
            for (int i=0; i<max_pipes; i++){
                if (pipes[i].used == NO){
                    //this is the empty pipe.
                    //writeShit("empty.pipe: ", 12); PL011_putc( UART0, '0' + i, true );PL011_putc( UART0, '\n', true );
                    index_pipe = i;
                    break;
                }
            }
            
            //GET ENDPOINT INDEX[s]
            int both = 0;
            for (int i=0; i<max_endpoints; i++){
                if (endpoints[i].used == NO){
                    //this is the empty endpoint.
                    if (index_ep1 == -1){
                        //update me.
                        //writeShit("empty.endp: ", 12); PL011_putc( UART0, '0' + i, true );PL011_putc( UART0, '\n', true );
                        index_ep1 = i;
                        both;
                    }
                    else {
                        //update ep2_ID
                        //writeShit("empty-endp: ", 12); PL011_putc( UART0, '0' + i, true );PL011_putc( UART0, '\n', true );
                        index_ep2 = i;
                        break;
                    }
                }
            }
            
            if (index_pipe != -1 && index_ep1 != -1 && index_ep2 != -1){
                //set pipe data.
                pipes[index_pipe].status = EMPTY;
                pipes[index_pipe].used = YES;
                //set endpoint_1 data.
                endpoints[index_ep1].pipeid = pipes[index_pipe].pipeid;
                endpoints[index_ep1].used = YES;
                endpoints[index_ep1].type = WRITE;
                //set endpoint_2 data.
                endpoints[index_ep2].pipeid = pipes[index_pipe].pipeid;
                endpoints[index_ep2].used = YES;
                endpoints[index_ep2].type = READ;
                //update fd.
                fd[0] = endpoints[index_ep1].eid;
                fd[1] = endpoints[index_ep2].eid;
                //return success.
                ctx->gpr[0] = 0;
            }
            else {
                ctx->gpr[0] = -1; //return error.
            }
            break;
        }
        case SEM_OPEN:{ //sem_init.
            int index = getNewSemaphore();
            ctx->gpr[0] = index;
            break;
        }
        case SEM_WAIT:{ //sem_post.
            int i = (int) ( ctx->gpr[ 0 ] );
            int value  = semaphores[i];
            if (value > 0){
                ctx->gpr[0] = value - 1;
            }
            else{
                ctx->gpr[0] = - 1;
            }
            
            break;
        }
        case SEM_POST: { //sem_post
            int i = (int) ( ctx->gpr[ 0 ] );
            int value  = semaphores[i];
            if (value >= 0){
                ctx->gpr[0] = value + 1;
            }
            else{
                ctx->gpr[0] = - 1;
            }
            break;
        }
        case DISPLAYON: {
            ctx->gpr[0]=0;
            break;
        }
        
        default   : { // 0x?? => unknown/unsupported
            break;
        }
    }

    return;
}

