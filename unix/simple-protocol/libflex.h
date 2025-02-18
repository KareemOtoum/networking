#include <stdint.h>

typedef uint8_t flx_act;
typedef uint8_t flx_opt;

#define FLX_ACT_PUSH 0x00
#define FLX_ACT_POP 0x01
#define FLX_ACT_PEEK 0x02
#define FLX_ACT_UNSET 0xFF

#define FLX_DLEN_PUSH 1
#define FLX_DLEN_POP 0
#define FLX_DLEN_PEEK 0

#define FLX_REPLY_VALID 0x00
#define FLX_REPLY_BAD_ACTION 0x01
#define FLX_REPLY_BAD_SIZE 0x02
#define FLX_REPLY_INVALID_DATA 0x03
#define FLX_REPLY_UNSET 0xFF

#define MAX_PKT_SIZE 255
#define MIN_PKT_SIZE 2

struct flx_msg 
{
    flx_act action;
    uint8_t size;

    char **data;
    int dataLen;
};

struct serialize_result
{
    int size;
    flx_opt reply;
};

void print_packet(struct flx_msg *msg);

void flex_msg_factory(struct flx_msg *msg);
void flex_msg_reset(struct flx_msg *msg);

void serialize_result_factory(struct serialize_result *result);

void serialize(uint8_t buffer[MAX_PKT_SIZE], struct flx_msg *msg, 
    struct serialize_result *result);

void deserialize(uint8_t buffer[MAX_PKT_SIZE], struct flx_msg *msg, 
    struct serialize_result *result);