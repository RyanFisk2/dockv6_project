# Final Report

## Overall project 
### Overall level: 2 + integration of 3 modules

## Module 1
### Module level: 3 + integration

- Test of level 0: added a print to cm.c to print out container specs from the container manager.
  -- Attribution: Ryan 

- Test of level 1: executed /ls as the container init process, also ran /ls with local and global paths within the container, correctly 
    printed only the files in the container directory
    
    -- Attribution: Ryan and Jon

- Test of level 2: ran nproc_test2 in the container, this tries to create up to 6 child processes, ran it in a container with nproc set to 4
    correctly ended the test after forking 4 times
    
    -- Attribution: Ryan
 
 - Test of level 3: ran nproc_test2 and /sh as the container init process, we were able to show correct constraints using ls and the nproc_tests
 
    -- Attribution: Ryan
    

## Module 2
### Module level: 3 + integration

- shmem_testp1 and shmem_testp2 test shared memory access across processes in different ancestries. This test is also used to test segregation of shared memory between global
    and within a container
    
    -- Attribution: Jon
  
- shmem2_p1 and shmem2_p2 test shared memory acccess across processes in different ancestries as well as between a parent and child proc. Also test memory inheritance across fork

    -- Attribution: Jon
    
 - shmem3 tests that shm_rem() properly removes a process' access to a shared page
 
    -- Attribution: Jon
    
 - shmem_test4 tests that sbrk and malloc work with shared memory and do not addect the shared page
 
    -- Attribution: Jon
    
 - shmem_test5 tests tracking and maintaining of shared mem pages across forks, exits, and calls to shm_rem. Test that processes still have access to a page they got after
 after other processes havce called shm_rem() on that page or exited while having page (also tests that exit correctly decrements ref count)
 
   -- Attribution: Jon
   
   
 ## Module 3
 ### Module level: 3 + integration
 
 - Mutex_test1 tests that mutexes prevent race conditions
 
    -- Attribution: Brian
    
 - Mutex_test2 tests cv_wait() and cv_signal()
 
    -- Attribution: Brian
    
 - Mutex_test3 tests error handling for the mutex system calls
 
    -- Attribution: Brian
    
 - Mutex_test4a and 4b test that we cannot create more than a set number of mutexes 
 
    -- Attribution: Brian
    
 - Mutex_test5 tests crash handling of a process while holding a mutex
 
    -- Attribution: Brian
    
 - Mutex_test6 test crash handling of condition variables when a process on cv_wait() crashes/gets killed
 
    -- Attribution: Brian
    
    
 ## Module 4
 ### Module level: 2
 
 - prio_set_test tests that a process cannot set the priority of a process if it is not in the process ancestry
 
   --Attribution: Jon
    
 - prio_set_test2 tests that a process cannot raise it's own or any other process's priority and that priorities are inherited on fork() and calling prio_set() on a non-existant
    process returns an error
    
    -- Attribution: Jon
    
- prio_set_test3 tests that a process can set another process' priority if it is in that process' ancestry

    -- Attribution: Jon
    
- sched_test1 tests the priority scheduler by having 2 processes with different priority levels get the same shared memory page and write different values to it.
   The number of tyimes an unexpected value is seen by the higher priority process (i.e. the number of preemptions by a lower priority process) is recorded and printed
   
   -- Attribution: Jon
   
- sched_test2 test that the priority scheduler equally considers processes of the same priority. This is the same as sched_test1 but with both processes having the same priority level

   -- Attribution: Jon
   

