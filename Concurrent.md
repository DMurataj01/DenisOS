# Demonstate Success criteria has been met.


# Stage 1
- Demonstate Concurrency
    - P3 & P4 automatically execute.
- Priority scheduler vs Round the robin execution..
    - Priority:
        - reset foundProgs index
        - for each NON-executing process, increment age.
        - *STUPID:* line 140 [ noOfProgs > 1]
        - LOOP AROUND PCB ARRAY:
            - if current program, set current_index.
            - if NON-executing process, found++, get priority and record if higher.
            - if all progs found, dispatch the highest & reset age of executed one.
         *STUPID:* line 170 [wtf is the return clause.]

    - ensures **ALL** processes get CPU time.

# Stage 2
- fork, exec and exit system calls.
    - reason about semantics if not adherent to POSIX!!
- these syscalls should work.

- Inter-Process Communication
    - Modified Semaphores... speak about them.
    - They are integers.
    - Dining Philosophers.
        - mutual exclusion.
        - there is no starvation.
    
# Stage 3
- LCD Display - Implementation of a GUI.

- **Denis Paint** - recreation of the recently deceased Microsoft Paint.
    - Mapping of keyboard characters to letters [function]
    - Left and Right click handlers.
        - distinguish between a click and a hold.
    - 



### Talk about extending the GUI.
