#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "libflex.h"

int main()
{
    uint8_t buffer[MAX_PKT_SIZE] =
    {
        FLX_ACT_PUSH,
        0xA,
        '1',
        '2',
        'Y',
        '\0',   
        'B',
        'O',
        '\0',
        'N',
        'O',
        '\0'
    };

    struct serialize_result *result = (struct serialize_result*)malloc(sizeof(struct serialize_result));
    serialize_result_factory(result);

    struct flx_msg *message = (struct flx_msg*) malloc(sizeof(struct flx_msg));
    flex_msg_factory(message);

    deserialize(buffer, message, result);

    if(result->reply != FLX_REPLY_VALID)
    {
        fprintf(stderr, "GOT %d\n", result->reply);
        return 1;
    }
    print_packet(message);

    char *lol = "hello there";

    char *data[1] =
    {
        lol
    };

    uint8_t cereal[MAX_PKT_SIZE] ={0};
    message->data = data;
    message->dataLen = 1;
    message->action = FLX_ACT_PUSH;
    message->size = 12;

    serialize(cereal, message, result);

    if(result->reply != FLX_REPLY_VALID)
    {
        fprintf(stderr, "GOT %d\n", result->reply);
        return 1;
    }

    for(int i = 0; i < 2 + cereal[1]; i++)
    {
        printf("%x ", cereal[i]);
    }

    
    return 0;
}