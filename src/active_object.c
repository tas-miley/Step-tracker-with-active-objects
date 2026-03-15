#include "active_object.h"

LOG_MODULE_REGISTER(common_ao, LOG_LEVEL_DBG);

void thread_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    LOG_INF("Spooling up thread.");
    active_object *ao = (active_object *) p1;
    ao_event evt;
    while (1) {
        LOG_DBG("Inside of %s AO thread.", k_thread_name_get(k_current_get()));
        k_msleep(1000);
        k_msgq_get(&ao->queue, &evt, K_FOREVER);
        k_msleep(1000);
        ao->dispatch(ao, &evt);
        // k_msleep(1000);
    }
}

void active_object_init(active_object *self, dispatch_func dispatch, int thread_prio, k_thread_stack_t *stack, size_t stack_size, uint8_t *buf, size_t queue_msg_size, int id) {
    LOG_INF("Initializing active object.");
    self->dispatch = dispatch;
    k_msgq_init(&self->queue, buf, queue_msg_size, MAX_QUEUE_MSGS);
    k_thread_create(&self->thread, stack, stack_size, &thread_entry, self, NULL, NULL, thread_prio, 0, K_NO_WAIT);
    self->id = id;
}

void ao_post(active_object *self, ao_event *evt) {
    int ret = k_msgq_put(&self->queue, evt, K_NO_WAIT);
    if (ret != 0) {
        LOG_ERR("Failed to put message for object %s", k_thread_name_get(k_current_get()));
    }
}