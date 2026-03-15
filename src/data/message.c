#include "data/message.h"

MESSAGE_CONTACT MESSAGE_CONTACT_make_blank() {
    const MESSAGE_CONTACT contact = { NULL, NULL, NULL };
    return contact;
}

MESSAGE_DATE MESSAGE_make_date() {
    const MESSAGE_DATE date = { 0, 0 ,0 ,0 , 0 };
    return date;
}

LIST_DEFINE(MESSAGE)

MESSAGE MESSAGE_make_blank() {
    const MESSAGE_CONTACT contact = MESSAGE_CONTACT_make_blank();
    const MESSAGE_DATE date = MESSAGE_make_date();
    const MESSAGE message = { contact, MESSAGE_BOX_UNKNOWN, date, NULL, 0 };
    return message;
}