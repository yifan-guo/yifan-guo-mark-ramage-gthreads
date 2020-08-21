#Summary
instead of implementing threads in kernel, We're implementing threads in user space

#Instructions to build the project
1. clone the repo
2. change into this directory
3. call "make"
4. run the test cases by invoking the following commands
    a. "./gtmain"
    b. "./gttest"
    c. "./gttest2"
    
For example, if running the gtmain command, you should see this output:

        Yifans-MacBook-Air:yifan-guo-mark-ramage-gthreads yifanguo$ ./gtmain 
        Running three threads
        Policy: FIFO thread id: 1 current iteration: 0
        Policy: FIFO thread id: 1 current iteration: 1
        Policy: FIFO thread id: 1 current iteration: 2
        Policy: FIFO thread id: 1 current iteration: 3
        Policy: FIFO thread id: 1 current iteration: 4
        Policy: FIFO thread id: 1 current iteration: 5
        Policy: FIFO thread id: 1 current iteration: 6
        Policy: FIFO thread id: 1 current iteration: 7
        Policy: FIFO thread id: 1 current iteration: 8
        Policy: FIFO thread id: 1 current iteration: 9
        Policy: Round Robin thread id: 2 current iteration: 0
        Policy: Round Robin thread id: 2 current iteration: 1
        Policy: Round Robin thread id: 2 current iteration: 2
        Policy: Round Robin thread id: 3 current iteration: 0
        Policy: Round Robin thread id: 3 current iteration: 1
        Policy: Round Robin thread id: 3 current iteration: 2
        Policy: Round Robin thread id: 2 current iteration: 3
        Policy: Round Robin thread id: 2 current iteration: 4
        Policy: Round Robin thread id: 2 current iteration: 5
        Policy: Round Robin thread id: 3 current iteration: 3
        Policy: Round Robin thread id: 3 current iteration: 4
        Policy: Round Robin thread id: 3 current iteration: 5
        Policy: Round Robin thread id: 2 current iteration: 6
        Policy: Round Robin thread id: 2 current iteration: 7
        Policy: Round Robin thread id: 2 current iteration: 8
        Policy: Round Robin thread id: 3 current iteration: 6
        Policy: Round Robin thread id: 3 current iteration: 7
        Policy: Round Robin thread id: 3 current iteration: 8
        Policy: Round Robin thread id: 2 current iteration: 9
        Policy: Round Robin thread id: 3 current iteration: 9
        Yifans-MacBook-Air:yifan-guo-mark-ramage-gthreads yifanguo$ 
        
        
        

#Instructions to use gthreads in any C code
1. include queue.c, uthash.c, and unistd.h
2. declare thread function
    a. must be of type void funcName (void) {}
    b. must include gtyield() when switching between gthreads
3. must declare itimer
4. initialize  gthreads by calling gtgo(), passing in appropriate parameters
5. call gtret(1) to start all gthreads and exit when done

