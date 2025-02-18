#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "libflex.h"

const int ActionSizes[3] = { FLX_DLEN_PUSH, FLX_DLEN_POP, FLX_DLEN_PEEK};

void print_packet(struct flx_msg *msg)
{
    printf("--PACKET--\n");
    printf("cmd: %x\nsize: %x\n", msg->action, msg->size);

    for(int i = 0; i < msg->dataLen; i++)
    {
        printf("data %d: %s\n", i, msg->data[i]);
    }
}

void serialize_result_factory(struct serialize_result *result)
{
    result->size = -1;
    result->reply = FLX_REPLY_UNSET;
}

void flex_msg_factory(struct flx_msg *msg)
{
    msg->action = FLX_ACT_UNSET;
    msg->size = 0;

    msg->data = NULL;
    msg->dataLen = 0;
}

void flex_msg_reset(struct flx_msg *msg)
{
    if(msg->data == NULL)
    {
        flex_msg_factory(msg);
        return;
    } 

    for(int i = 0; i < msg->dataLen; i++)
    {
        if(msg->data[i] != NULL)
        {
            free(msg->data[i]);
        }
    }

    free(msg->data);
    flex_msg_factory(msg);
}

void serialize(uint8_t buffer[MAX_PKT_SIZE], struct flx_msg *msg, 
    struct serialize_result *result)
{
    int validDataLength = -1;
    int dataSize = 0;
    int runningDataSize = 0;

    serialize_result_factory(result);

    if(msg->action <= ActionSizes[2])
    {
        buffer[0] = msg->action;
        validDataLength = ActionSizes[msg->action];
    } else
    {
       result->reply = FLX_REPLY_BAD_ACTION;
       return; 
    }

    if(validDataLength != msg->dataLen)
    {
        result->reply = FLX_REPLY_BAD_SIZE;
        return;
    }

    if(validDataLength != 0 && msg->data == NULL)
    {
        result->reply = FLX_REPLY_INVALID_DATA;
        return;
    }

    for(int i = 0; i < msg->dataLen; i++)
    {
        if(msg->data[i] == NULL)
        {
            result->reply = FLX_REPLY_INVALID_DATA;
            return;
        }

        for(int j = 0; j < MAX_PKT_SIZE - MIN_PKT_SIZE - dataSize; j++)
        {
            runningDataSize += 1;
            buffer[MIN_PKT_SIZE + dataSize + j] = msg->data[i][j];

            if(msg->data[i][j] == '\0')
            {
                break;
            }
        }

        dataSize += runningDataSize;
        runningDataSize = 0;
    }

    buffer[1] = dataSize;
    result->size = MIN_PKT_SIZE + dataSize;
    result->reply = FLX_REPLY_VALID;
    return;
}

void deserialize(uint8_t buffer[MAX_PKT_SIZE], struct flx_msg *msg,
    struct serialize_result *result)
{
    int validDataLength = -1;
    int dataSizeIndex = 0;
    int dataSize = 0;
    int *dataSizes = NULL;
    int dataOffset = MIN_PKT_SIZE;

    serialize_result_factory(result);
    flex_msg_reset(msg);

    if(buffer[0] <= ActionSizes[2])
    {
        msg->action = buffer[0];
        validDataLength = ActionSizes[msg->action];
    } else
    {
        result->reply = FLX_REPLY_BAD_ACTION;
        return;
    }

    if(validDataLength == 0)
    {
        if(buffer[1] != 0)
        {
            result->reply = FLX_REPLY_BAD_SIZE;
        } else
        {
            result->reply = FLX_REPLY_VALID;
        }
        return;
    }

    msg->size = buffer[1];
    dataSizes = (int *)malloc(sizeof(int) * validDataLength);

    for(int i = MIN_PKT_SIZE; i < MAX_PKT_SIZE; i++)
    {
        if(i > msg->size + MIN_PKT_SIZE)
        {
            free(dataSizes);
            result->reply = FLX_REPLY_BAD_SIZE;
            return;
        }

        if(dataSizeIndex == validDataLength)
        {
            if(dataSize == 1)
            {
                free(dataSizes);
                result->reply = FLX_REPLY_BAD_SIZE;
            }
            break;
        }

        if(buffer[i] == '\0')
        {
            dataSizes[dataSizeIndex] = ++dataSize;
            dataSizeIndex++;
            dataSize = 0;
            continue;
        }

        if(buffer[i] < '!' || buffer[i] > '~')
        {
            free(dataSizes);
            result->reply = FLX_REPLY_INVALID_DATA;
            return;
        }

        dataSize++;
    }

    msg->data = (char**)malloc(sizeof(char *) * validDataLength);
    msg->dataLen = validDataLength;

    for(int i = 0; i < validDataLength; i++)
    {
        msg->data[i] = (char *)malloc(sizeof(char) * dataSizes[i]);

        for(int j = 0; j < dataSizes[i]; j++)
        {
            msg->data[i][j] = buffer[j + dataOffset];
        }
        dataOffset += dataSizes[i];
    }

    result->reply = FLX_REPLY_VALID;
    result->size = MIN_PKT_SIZE + msg->size;
    free(dataSizes);
    return;
}