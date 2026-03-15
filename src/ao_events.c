#include "ao_events.h"

LOG_MODULE_REGISTER(ao_events, LOG_LEVEL_DBG);

static active_object *subscriptions[MAX_EVENTS][MAX_SUBSCRIPTIONS];
static int sub_count[MAX_EVENTS];

void ao_subscribe(active_object *self, ao_event_id evt_id) {
    // keep track of ao that is subscribed to this event.
    LOG_INF("EVENT ID %d", evt_id);
    subscriptions[evt_id][sub_count[evt_id]] = self;
    sub_count[evt_id]++;
}

void ao_unsubscribe(active_object *self, ao_event_id evt_id) {
    int cnt = sub_count[evt_id];
    for (int i = 0; i < cnt; i++) {
        if (subscriptions[evt_id][i] == self) {
            // swap with last and decrement, avoid array resizing.
            subscriptions[evt_id][i] = subscriptions[evt_id][cnt - 1];
            subscriptions[evt_id][cnt - 1] = NULL;
            sub_count[evt_id]--;
            return;
        }
    }
}

void ao_publish(ao_event *evt) {
    int cnt = sub_count[evt->id];
    for (int i = 0; i < cnt; i++) {
        ao_post(subscriptions[evt->id][i], evt);
    } 
}