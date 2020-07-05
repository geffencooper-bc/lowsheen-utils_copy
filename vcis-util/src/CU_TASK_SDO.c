
#include "CU.h"
#include "CANopen.h"
#include "CO_OD.h"
#include <stdio.h>

typedef enum
{
    CU_TASK_SDO_PROCESS_READY = 0,              // ready for processing any sdo task
    CU_TASK_SDO_PROCESS_WAIT_DOWNLOAD,      // sdo download task running, waiting for results
    CU_TASK_SDO_PROCESS_WAIT_UPLOAD,        // sdo upload task running, waiting for results
}CU_TASK_SDO_PROCESS;


/* sdo transfer helpers */
CU_TASK_STATUS CU_TASK_SDO_download(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len)
{
    const uint8_t blockTransferEnable = 1;
    /* Setup client. */
    if(CO_SDOclient_setup(CO->SDOclient[0], 0, 0, cmd->nodeID) != CO_SDOcli_ok_communicationEnd)
    {
        printf("Failed to setup SDO client");
        return CU_TASK_STATUS_ERROR;
    }

    if(CO_SDOclientDownloadInitiate(CO->SDOclient[0], cmd->sdoIndex, cmd->sdoSubIndex, data,
                    data_len, blockTransferEnable) != CO_SDOcli_ok_communicationEnd)
    {
        printf("Failed to initiate SDO client download");
        return CU_TASK_STATUS_ERROR;
    }

    return CU_TASK_STATUS_NO_ERROR;
}

CU_TASK_STATUS CU_TASK_SDO_upload(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len)
{
    const uint8_t blockTransferEnable = data_len > 4;
    /* Setup client. */
    if(CO_SDOclient_setup(CO->SDOclient[0], 0, 0, cmd->nodeID) != CO_SDOcli_ok_communicationEnd)
    {
        printf("Failed to setup SDO client");
        return CU_TASK_STATUS_ERROR;
    }

    if(CO_SDOclientUploadInitiate(CO->SDOclient[0], cmd->sdoIndex, cmd->sdoSubIndex, data,
                    data_len, blockTransferEnable) != CO_SDOcli_ok_communicationEnd)
    {
        printf("Failed to initiate SDO client upload\r\n");
        return CU_TASK_STATUS_ERROR;
    }

    return CU_TASK_STATUS_NO_ERROR;

}

CU_TASK_STATUS CU_TAKS_SDO_downloadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *abort_code)
{
    int ret;

    ret = CO_SDOclientDownload(CO->SDOclient[0], time_diff_1ms, 10000, abort_code);

    if(ret <= 0)
    {   
        CO_SDOclientClose(CO->SDOclient[0]);

        if(ret == CO_SDOcli_ok_communicationEnd)
        {
            return CU_TASK_STATUS_DONE;
        }
        else if(ret == CO_SDOcli_endedWithServerAbort)
        {
            return CO_SDOcli_endedWithClientAbort;
        }
        else if(ret == CO_SDOcli_endedWithTimeout)
        {
            return CO_SDOcli_endedWithTimeout;
        }
        else
        {
            return CU_TASK_STATUS_ERROR;
        }
    }
    else
    {
        return CU_TASK_STATUS_CONTINUE;
    }
}

CU_TASK_STATUS CU_TASK_SDO_uploadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *data_read_len, uint32_t *abort_code)
{
    int ret;


    ret = CO_SDOclientUpload(CO->SDOclient[0], time_diff_1ms, 500, data_read_len, abort_code);

    if(ret <= 0)
    {
        CO_SDOclientClose(CO->SDOclient[0]);

        if(ret == CO_SDOcli_ok_communicationEnd)
        {
            return CU_TASK_STATUS_DONE;
        }
        else if(ret == CO_SDOcli_endedWithServerAbort)
        {
            return CU_TASK_STATUS_ABORT;
        }
        else if(ret == CO_SDOcli_endedWithTimeout)
        {
            return CU_TASK_STATUS_TIMEOUT;
        }
        else
        {
            return CU_TASK_STATUS_ERROR;
        }
    }
    else
    {
        return CU_TASK_STATUS_CONTINUE;
    }
}

// avoids having repetitive req/wait for sdo messages, keep polling function until done
CU_TASK_STATUS CU_TASK_SDO_downloadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *abort_code)
{
    CU_TASK_STATUS status;

    switch(cmd->sdo_process)
    {

        case CU_TASK_SDO_PROCESS_READY:

            cmd->nodeID = node_id;
            cmd->sdoIndex = index;
            cmd->sdoSubIndex = sub_index;

            status = CU_TASK_SDO_download(cmd, data,  data_len);

            if(status != CU_TASK_STATUS_NO_ERROR)
            {
                return status;
            }
            else
            {
                cmd->sdo_process = CU_TASK_SDO_PROCESS_WAIT_DOWNLOAD;
                return CU_TASK_STATUS_CONTINUE; // report that the proces is on going
            }

        case CU_TASK_SDO_PROCESS_WAIT_DOWNLOAD:

            status = CU_TAKS_SDO_downloadUpdate(cmd, time_diff_1ms, abort_code);

            switch(status)
            {
                case CU_TASK_STATUS_CONTINUE:
                    break;
                default:
                    // either complete, or error; either way sdo process done
                    cmd->sdo_process = CU_TASK_SDO_PROCESS_READY;
            }

            return status;

        default:

            cmd->sdo_process = CU_TASK_SDO_PROCESS_READY;

            return CU_TASK_STATUS_ERROR;
    }

}


CU_TASK_STATUS CU_TASK_SDO_uploadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *data_read_len, uint32_t *abort_code)
{
    CU_TASK_STATUS status;

    switch(cmd->sdo_process)
    {

        case CU_TASK_SDO_PROCESS_READY:

            cmd->nodeID = node_id;
            cmd->sdoIndex = index;
            cmd->sdoSubIndex = sub_index;

            status = CU_TASK_SDO_upload(cmd, data,  data_len);

            if(status != CU_TASK_STATUS_NO_ERROR)
            {
                return status;
            }
            else
            {
                cmd->sdo_process = CU_TASK_SDO_PROCESS_WAIT_UPLOAD;
                return CU_TASK_STATUS_CONTINUE; // report that the process is on going
            }

        case CU_TASK_SDO_PROCESS_WAIT_UPLOAD:

            status = CU_TASK_SDO_uploadUpdate(cmd, time_diff_1ms, data_read_len, abort_code);

            switch(status)
            {
                case CU_TASK_STATUS_CONTINUE:
                    break;
                default:
                    // either complete, or error, either way, sdo process done
                    cmd->sdo_process = CU_TASK_SDO_PROCESS_READY;
            }

            return status;

        default:

            cmd->sdo_process = CU_TASK_SDO_PROCESS_READY;

            return CU_TASK_STATUS_ERROR;
    }

}