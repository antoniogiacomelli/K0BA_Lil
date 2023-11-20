/**
 * @file kernel.h
 * @brief Header K0BA Lite - Kernel 0 for emBedded Arm
 * @date 18/11/23
 * @author Antonio Giacomelli <antoniogiacomelli@protonmail.com>
 * Copyright (C) 2023 www.antoniogiacomelli.com
 */
#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_
#include <stdint.h>

/** @brief Kernel version string */
#define KVERSION "0.1L"


/************************************************************************/
/* Configuration Macros                                                 */
/************************************************************************/

#define _NTHREADS 4 /**< Total number of application threads */
#define NTHREADS (_NTHREADS + 1)  /**< Number of threads including TaskIdle */
#define STACK_SIZE 256 /** Stack size for each thread */
#define OK	0 /**< Return code for success */
#define NOK	-1 /**< Return code for failure */
#define MSG_SIZE 64 /**< Size for each message buffer for Message Passsing */
#define FIFO_SIZE 64 /**< FIFO queue size */
#define NMBUF	5 /**< Number of Message Buffers on the system */
#define PSIZE 64 /**< Pipe message size */
#define NPIPE 5 /**< Number of total pipes on the system */

/************************************************************************/
/* Thread Management and ITC                                            */
/************************************************************************/

// Typedefs
//********************
typedef unsigned int size_t;
typedef enum {
    READY, /**< Thread is ready to run */
    RUNNING, /**< Thread is currently running */
    SLEEPING, /**< Thread is in a sleep state */
    BLOCKED /**< Thread is blocked */
} thread_status_t;

typedef struct tcb TCB_t; /**< Thread Control Block struct */
typedef void (*Task)(void*); /**< Task function pointer */
typedef volatile uint8_t MSG_t; /**< Volatile character for messages */
typedef volatile int32_t EVENT_t; /**< Volatile integer for events */

/**
 * @brief Blocked Queue structure used by mutexes and semaphores
 * to track blocked threads and their order.
 */
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint8_t init;
    TCB_t* blocked[NTHREADS];
} __attribute__((aligned)) BQUEUE_t;

/**
 * @brief Counting Semaphore structure containing a value and a blocked queue.
 * Value indicates the number of threads blocked (if negative).
 * If positive, the number of free slots
 */
typedef struct {
    int32_t value;
    BQUEUE_t queue;
} __attribute__((aligned)) SEMA_t;

/**
 * @brief Mutex structure holding a blocked queue, lock status, and owner thread.
 */
typedef struct {
    BQUEUE_t queue;
    uint8_t lock; /**< 0=unlocked, 1=locked */
    TCB_t* owner; /**< Pointer to current owner */
} __attribute__((aligned)) MUTEX_t;

/**
 * @brief FIFO Queue structure.
 */
typedef struct {
    uint32_t head;
    uint32_t tail;
    SEMA_t size;
    SEMA_t roomleft;
    MUTEX_t mutex;
    uint8_t buffer[FIFO_SIZE];
} __attribute__((aligned)) FIFO_t;

/**
 * @brief Pipe structure for inter-task communication.
 */
typedef struct {
    int32_t head;
    int32_t tail;
    int32_t data;
    int32_t room;
    int32_t status;
    uint8_t buf[PSIZE];
} __attribute__((aligned)) PIPE_t;

/**
 * @brief Message Buffer structure for Message Passing.
 */
typedef struct mbuff {
    struct mbuff* next;
    int sender_pid;
    uint8_t contents[MSG_SIZE];
} __attribute__((aligned)) MBUFF_t;

/**
 * @brief Thread Control Block (TCB) structure representing each thread.
 */
struct __attribute__((aligned)) tcb {
    uint32_t* sp; /**< Last saved task stack */
    struct tcb* next; /**< Next TCB */
    uint8_t pid; /**< task ID */
    uint32_t* block_sema; /**< Blocked semaphore */
    uint32_t* block_mutex; /**< Blocked mutex */
    uint32_t sleeping; /**< Sleep counter */
    SEMA_t nmsg; /**< New message semaphore */
    SEMA_t mlock; /**< Blocking semaphore for message queue */
    MBUFF_t* mqueue; /**< Message queue */
    uint8_t priority; /**< Priority: 0=highest */
    uint8_t rpriority;/**< Saved real priority in case of priority inheritance*/
    uint32_t event; /**< Sleep/wake event */
    thread_status_t status; /**< Thread status */
};

/* Globals */
extern TCB_t tcbs[NTHREADS];/**< Task Control Block */
extern uint32_t p_stacks[NTHREADS][STACK_SIZE]; /**< Thread stack */
extern TCB_t* RunPtr; /**< Pointer to the running thread */
extern MBUFF_t mbuff[NMBUF]; /**< Total message buffers on the system*/
extern MBUFF_t* mbufflist; /**< Free message buffers */
extern SEMA_t nmbuf; /**< Counting semaphore for the number of available 
                           Message buffers */
extern SEMA_t mlock; /**< Mutex to protect CR when getting/putting MBUFFs */
extern PIPE_t pipe[NPIPE]; /**< Pool of pipes on the system */

/* Kernel Prototypes */
/**
 * @defgroup Kernel_Functions Kernel Functions
 * @brief K0BA API
 * @{
 */

/**
 * @brief Starts the kernel.
 */
extern void kStart(void);

/**
 * @brief Sets the initial stack for a task.
 * @param i Task index
 */
extern void kSetInitStack(uint8_t i);

/**
 * @brief Performs a task switch.
 */
extern void kTaskSwitch(void);

/**
 * @brief Adds a task to the kernel.
 * @param t Pointer to the task function
 * @param args Arguments for the task function
 * @param pid  ID of the task
 * @param priority Priority of the task
 * @return Return code indicating success (OK) or failure (NOK)
 */
extern int8_t kAddTask(Task t, void *args, uint8_t pid, uint8_t priority);

/**
 * @brief Yields the processor to another task.
 *        (Triggers PendSV)
 */
extern void kYield(void);

/**
 * @brief Initializes all message buffers as free/empty.
 */
extern void kMbufInitAll(void);

/**
 * @brief Sends a synchronous message to a specified task.
 * @param msg Message to be sent
 * @param pid task ID of the receiver
 * @return Return code indicating success (OK) or failure (NOK)
 */
extern int8_t kSendMsg(const uint8_t *msg, uint8_t pid);

/**
 * @brief Receives a synchronous message.
 * @param msg Buffer to store the received message
 * @return Returns sender PID, if success, NOK if fail.
 */
extern int8_t kRcvMsg(uint8_t *msg);

/**
 * @brief Initializes all pipes.
 */
extern void kPipeInitAll(void);

/**
 * @brief Creates a new pipe.
 * @return Pointer to the created pipe
 */
extern PIPE_t *kPipeCreate(void);

/**
 * @brief Reads from a pipe.
 * @param p Pointer to the pipe
 * @param buf Buffer to store the read data
 * @param n Number of bytes to read
 * @return Number of bytes read
 */
extern int32_t kPipeRead(PIPE_t *p, uint8_t *buf, size_t n);

/**
 * @brief Writes to a pipe.
 * @param p Pointer to the pipe
 * @param buf Buffer containing data to write
 * @param n Number of bytes to write
 * @return Number of bytes written
 */
extern uint32_t kPipeWrite(PIPE_t *p, uint8_t *buf, size_t n);

/**
 * @brief Signals a semaphore.
 * @param s Pointer to the semaphore
 */
extern void kSemaSignal(SEMA_t* s);

/**
 * @brief Waits on a semaphore.
 * @param s Pointer to the semaphore
 */
extern void kSemaWait(SEMA_t* s);

/**
 * @brief Initializes a semaphore with a given value.
 * @param s Pointer to the semaphore
 * @param value Initial value of the semaphore
 */
extern void kSemaInit(SEMA_t* s, int32_t value);

/**
 * @brief Forces a sleeping task to wake-up
 * @param pid Task ID
 */
extern void kWakeTicks(uint32_t pid);

/**
 * @brief Puts the current task to sleep for a specified number of ticks.
 * @param ticks Number of ticks to sleep
 */
extern void kSleepTicks(uint32_t ticks);

/**
 * @brief Puts the current task to sleep based on an event.
 * @param event Event identifier
 */
extern void kSleep(EVENT_t event);

/**
 * @brief Wakes up the task waiting for a specific event.
 * @param event Event identifier
 */
extern void kWake(EVENT_t event);

/**
 * @brief Sends a message asynchronously to a specified task.
 * @param msg Message to be sent
 * @param pid task ID of the receiver
 * @return Return code indicating success (OK) or failure (NOK)
 */
extern int8_t kAsendMsg(uint8_t* msg, uint8_t pid);

/**
 * @brief Receives a message asynchronously.
 * @param msg Buffer to store the received message
 * @return Return sender PID if success, NOK if fail.
 */
extern int8_t kArecvMsg(uint8_t *msg);
/**
 * @brief Disables task switching.
 */
extern void kDisableTaskSwitch(void);

/**
 * @brief Enables task switching.
 */
extern void kEnableTaskSwitch(void);

/**
 * @brief Initializes a FIFO queue.
 * @param me Pointer to the FIFO queue
 */
extern void FifoInit(FIFO_t* me);

/**
 * @brief Puts data into a FIFO queue.
 * @param me Pointer to the FIFO queue
 * @param data Data to put into the FIFO queue
 * @return Return code indicating success (OK) or failure (NOK)
 */
extern int32_t FifoPut(FIFO_t* me, uint8_t data);
/**
 * @brief Gets data from a FIFO queue
 * @param me Pointer to the FIFO queue
 * @return Data retrieved from the FIFO queue
 */
extern uint8_t FifoGet(FIFO_t* me);

/**
 * @brief Locks a mutex.
 * @param m Pointer to the mutex
 */
extern void kLock(MUTEX_t* m);

/**
 * @brief Releases a mutex.
 * @param m Pointer to the mutex
 */
extern void kRelease(MUTEX_t* m);

/**
 * @brief Initializes a mutex.
 * @param m Pointer to the mutex
 * @param value Initial value of the mutex
 */
extern void kMutexInit(MUTEX_t* m, int value);


/**
 * @}
 */ // End of Kernel_Functions group

#endif /* INC_KERNEL_H_ */
