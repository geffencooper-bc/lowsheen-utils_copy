

#include <stdint.h>


// canopen-util --interface can0 --node 0x49 --program_file ./curtis_trac.bin --program 1 --info --reset

typedef struct
{
    uint8_t nodeID;
    uint16_t sdoIndex;
    uint8_t sdoSubIndex;
    char *interfaceName;
    uint8_t programID;
    char *programFilename;
    uint8_t *programData;
    uint32_t programDataLen;
    uint8_t sdo_process;
    uint8_t step;
    uint8_t index;
}CU_TaskDetails;

typedef enum
{   
    CU_TASK_STATUS_NO_ERROR,
    CU_TASK_STATUS_PENDING,
    CU_TASK_STATUS_CONTINUE,
    CU_TASK_STATUS_DONE,
    CU_TASK_STATUS_TIMEOUT,
    CU_TASK_STATUS_ABORT,
    CU_TASK_STATUS_ERROR,
}CU_TASK_STATUS;

/* parse command line entries */
void CU_COMMAND_parse(CU_TaskDetails *cmd, int argc, char *argv[]);

/* task invocation */
void CU_TASK_addTask(const char *request_task_name, int32_t option);
CU_TASK_STATUS CU_TASK_update(CU_TaskDetails *task_details, uint32_t time_diff_1ms);
 
/* sdo transfer helpers */
CU_TASK_STATUS CU_TASK_SDO_download(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len);
CU_TASK_STATUS CU_TAKS_SDO_downloadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *abort_code);
CU_TASK_STATUS CU_TASK_SDO_upload(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len);
CU_TASK_STATUS CU_TASK_SDO_uploadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *data_read_len, uint32_t *abort_code);

CU_TASK_STATUS CU_TASK_SDO_downloadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *abort_code);
CU_TASK_STATUS CU_TASK_SDO_uploadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *data_read_len, uint32_t *abort_code);

/* request and display info task */
void CU_TASK_INFO_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_INFO_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* program update task */
void CU_TASK_PROGRAM_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_PROGRAM_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* request exit back to default application*/
void CU_TASK_RESET_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_RESET_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

void CU_COMMAND_parseArgs(CU_TaskDetails *cmd, int argc, char *argv[]);