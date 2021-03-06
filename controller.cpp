#include "controller.h"

/*___________________________________________________________________________*/

static uint8_t pulse_finished_handler(struct event_t *event)
{
    const uint8_t relay_port = CONTAINER_OF(event, door_t, event)->relay_port;

    CLR_BIT(PORTC, 1 << relay_port);

    return CANIOT_OK;
}

void GarageDoorController::actuate(door_t &door)
{
    PORTC |= 1 << door.relay_port;

    schedule(door.event, RELAY_PULSE_DURATION_MS);
}

// we suppose that the doors are not moving down or up
void GarageDoorController::poll_doors_status(void)
{
    const uint8_t inputs = read_inputs();
    for (uint_fast8_t idx = 0; idx < ARRAY_SIZE(doors); idx++) {
        door_state_t previous = doors[idx].state;
        doors[idx].state = (door_state_t) ((inputs >> doors[idx].inx) & 1);

        /* on state change, send a new telemetry message */
        if (doors[idx].state != previous) {
            request_telemetry();
        }
    }
}

/*___________________________________________________________________________*/

void GarageDoorController::initialize(void)
{
    CustomBoard::initialize();

    config.set_telemetry_period(TELEMETRY_PERIOD);
    config.data->telemetry_rdm_delay = SEC_TO_MS(3);
    config.data->telemetry_min = 500;

    set_command_handler(command_handler);
    set_telemetry_builder(telemetry_builder);

    doors[LEFT].event.handler = pulse_finished_handler;
    doors[RIGHT].event.handler = pulse_finished_handler;

    poll_doors_status();

    /* clear telemetry flag if set by poll_doors_status */
    CLEAR_FLAG_TELEMETRY(flags);

    /* send telemetry on start up */
    schedule_random_telemetry();
}

uint8_t GarageDoorController::command_handler(uint8_t buffer[8], uint8_t len)
{
    GarageDoorController * ctrl = (GarageDoorController*) get_instance();
    if (AS(buffer, CRTAAA_t)->relays.r1) {
        ctrl->actuate_left();
    }
    if (AS(buffer, CRTAAA_t)->relays.r2) {
        ctrl->actuate_right();
    }
    return CANIOT_OK;
}

uint8_t GarageDoorController::telemetry_builder(uint8_t buffer[8], uint8_t &len)
{
    GarageDoorController * ctrl = (GarageDoorController*) get_instance();

    AS(buffer, CRTAAA_t)->contacts.c1 = ctrl->doors[LEFT].state;
    AS(buffer, CRTAAA_t)->contacts.c2 = ctrl->doors[RIGHT].state;
    AS(buffer, CRTAAA_t)->contacts.c3 = ctrl->doors[GATE].state;
    AS(buffer, CRTAAA_t)->temperature = ctrl->read_temperature();
    len = CANIOT_GET_LEN(CRTAAA);

    return CANIOT_OK;
}