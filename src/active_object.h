#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define MAX_QUEUE_MSGS 10


typedef struct active_object_event {
    uint32_t id;
    uint32_t data;
} ao_event;

// function pointer to the active object's internal state machine dispatch func.
typedef void (*dispatch_func)(void *self, ao_event const *evt);

typedef struct active_object {
    int id; 
    k_tid_t tid; // easy access for use in zephyr threading api
    struct k_thread thread;
    struct k_msgq queue;
    dispatch_func dispatch;
    void *self;
} active_object;

void active_object_init(active_object *self,
                        dispatch_func dispatch,
                        int thread_prio,
                        k_thread_stack_t *stack,
                        size_t stack_size,
                        uint8_t *buf,
                        size_t queue_msg_size,
                        int id);

void ao_post(active_object *self, const ao_event *evt);
