/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <limits.h>
#include "sample_comm.h"
#include "autoconf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define TEMP_BUF_LEN    8
#define PER_UNIT        1024
#define MAX_THM_SIZE    (64 * 1024)

static pthread_t gs_VencPid;
static pthread_t gs_VencQpmapPid;
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
static SAMPLE_VENC_QPMAP_SENDFRAME_PARA_S stQpMapSendFramePara;
static HI_S32 gs_s32SnapCnt = 0;

static HI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, HI_CHAR *szFilePostfix, HI_U8 len)
{
    if (szFilePostfix == NULL) {
        SAMPLE_PRT("null pointer\n");
        return HI_FAILURE;
    }

    if (PT_H264 == enPayload) {
        if (strcpy_s(szFilePostfix, len, ".h264") != EOK) {
            return HI_FAILURE;
        }
    } else if (PT_H265 == enPayload) {
        if (strcpy_s(szFilePostfix, len, ".h265") != EOK) {
            return HI_FAILURE;
        }
    } else if (PT_JPEG == enPayload) {
        if (strcpy_s(szFilePostfix, len, ".jpg") != EOK) {
            return HI_FAILURE;
        }
    } else if (PT_MJPEG == enPayload) {
        if (strcpy_s(szFilePostfix, len, ".mjp") != EOK) {
            return HI_FAILURE;
        }
    } else {
        SAMPLE_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_VOID *SAMPLE_RTSP_VENC_GetVencStreamProc(HI_VOID *p)
{
    printf("[DBG] Using rtsp_venc_lib.\r\n");
    HI_S32 i;
    HI_S32 s32ChnTotal;
    VENC_CHN_ATTR_S stVencChnAttr;
    SAMPLE_VENC_GETSTREAM_PARA_S *pstPara = HI_NULL;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_U32 u32PictureCnt[VENC_MAX_CHN_NUM] = {0};
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][FILE_NAME_LEN];
    HI_CHAR real_file_name[VENC_MAX_CHN_NUM][PATH_MAX];
    FILE* pFile[VENC_MAX_CHN_NUM] = {HI_NULL};
    HI_S32 fd[VENC_MAX_CHN_NUM] = {0};
    char szFilePostfix[10]; /* length set 10 */
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
    VENC_STREAM_BUF_INFO_S stStreamBufInfo[VENC_MAX_CHN_NUM];

    prctl(PR_SET_NAME, "GetVencStream", 0, 0, 0);

    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S *)p;
    s32ChnTotal = pstPara->s32Cnt;
    if (s32ChnTotal >= VENC_MAX_CHN_NUM) {
        SAMPLE_PRT("input count invalid\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++) {
        /* decide the stream file name, and open file to save stream */
        VencChn = pstPara->VeChn[i];
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if (s32Ret != HI_SUCCESS) {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", VencChn, s32Ret);
            return NULL;
        }
        enPayLoadType[i] = stVencChnAttr.stVencAttr.enType;

        s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix, sizeof(szFilePostfix));
        if (s32Ret != HI_SUCCESS) {
            SAMPLE_PRT("SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", stVencChnAttr.stVencAttr.enType,
                s32Ret);
            return NULL;
        }
        // if (PT_JPEG != enPayLoadType[i]) {
#ifndef OHOS_CONFIG_WITHOUT_SAVE_STREAM
            // if (snprintf_s(aszFileName[i], FILE_NAME_LEN, FILE_NAME_LEN - 1, "./") < 0) {
            //     return NULL;
            // }
            // if (realpath(aszFileName[i], real_file_name[i]) == NULL) {
            //     SAMPLE_PRT("chn %d stream path error\n", VencChn);
            // }

            // if (snprintf_s(real_file_name[i], FILE_NAME_LEN, FILE_NAME_LEN - 1, "stream_chn%d%s", i, szFilePostfix) <
            //     0) {
            //     return NULL;
            // }

            // fd[i] = open(real_file_name[i], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            // if (fd[i] < 0) {
            //     SAMPLE_PRT("open file[%s] failed!\n", real_file_name[i]);
            //     return NULL;
            // }

            // pFile[i] = fdopen(fd[i], "wb");
            // if (pFile[i] == HI_NULL) {
            //     SAMPLE_PRT("fdopen error!\n");
            //     close(fd[i]);
            //     return NULL;
            // }
#endif
        // }
        VencFd[i] = HI_MPI_VENC_GetFd(VencChn);
        if (VencFd[i] < 0) {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i]) {
            maxfd = VencFd[i];
        }

        s32Ret = HI_MPI_VENC_GetStreamBufInfo(VencChn, &stStreamBufInfo[i]);
        if (s32Ret != HI_SUCCESS) {
            SAMPLE_PRT("HI_MPI_VENC_GetStreamBufInfo failed with %#x!\n", s32Ret);
            return (void *)HI_FAILURE;
        }
    }

    while (HI_TRUE == pstPara->bThreadStart) {
#ifndef CONFIG_USER_SPACE
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++) {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec = 2; /* 2 s */
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) {
            SAMPLE_PRT("select failed!\n");
            break;
        } else if (s32Ret == 0) {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        } else {
            for (i = 0; i < s32ChnTotal; i++) {
                if (FD_ISSET(VencFd[i], &read_fds)) {
#else
                for (i = 1; i < s32ChnTotal; i++) { // change from 0 -> 1: only stream chn[1]
#endif
                    (HI_VOID)memset_s(&stStream, sizeof(stStream), 0, sizeof(stStream));

                    VencChn = pstPara->VeChn[i];

                    s32Ret = HI_MPI_VENC_QueryStatus(VencChn, &stStat);
                    if (s32Ret != HI_SUCCESS) {
                        SAMPLE_PRT("HI_MPI_VENC_QueryStatus chn[%d] failed with %#x!\n", VencChn, s32Ret);
                        break;
                    }
                    if (stStat.u32CurPacks == 0) {
#ifdef CONFIG_USER_SPACE
                        usleep(10 * 1000);
#endif
                        continue;
                    }
                    stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (stStream.pstPack == NULL) {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(VencChn, &stStream, HI_TRUE);
                    if (s32Ret != HI_SUCCESS) {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                        break;
                    }
                    HisiPutH264DataToBuffer(&stStream);
#ifdef OHOS_CONFIG_WITHOUT_SAVE_STREAM
                    static int once = 0;
                    if (once == 0) {
                        printf("get first frame pts:%llu!\n", stStream.pstPack[0].u64PTS);
                        once = 1;
                    }
#endif
                    // if (PT_JPEG == enPayLoadType[i]) {
                    //     if (strcpy_s(szFilePostfix, sizeof(szFilePostfix), ".jpg") != EOK) {
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         return HI_NULL;
                    //     }
                    //     if (snprintf_s(aszFileName[i], FILE_NAME_LEN, FILE_NAME_LEN - 1, "./") < 0) {
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         return NULL;
                    //     }
                    //     if (realpath(aszFileName[i], real_file_name[i]) == NULL) {
                    //         SAMPLE_PRT("chn %d stream path error\n", VencChn);
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         return NULL;
                    //     }

                    //     if (snprintf_s(real_file_name[i], FILE_NAME_LEN, FILE_NAME_LEN - 1, "stream_chn%d_%d%s",
                    //         VencChn, u32PictureCnt[i], szFilePostfix) < 0) {
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         return NULL;
                    //     }

                    //     fd[i] = open(real_file_name[i], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    //     if (fd[i] < 0) {
                    //         SAMPLE_PRT("open file[%s] failed!\n", real_file_name[i]);
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         return NULL;
                    //     }

                    //     pFile[i] = fdopen(fd[i], "wb");
                    //     if (!pFile[i]) {
                    //         free(stStream.pstPack);
                    //         stStream.pstPack = NULL;
                    //         SAMPLE_PRT("fdopen err!\n");
                    //         close(fd[i]);
                    //         return NULL;
                    //     }
                    // }
#ifndef OHOS_CONFIG_WITHOUT_SAVE_STREAM
#if (!defined(__HuaweiLite__)) || defined(__OHOS__)
                    // s32Ret = SAMPLE_RTSP_VENC_SaveStream(pFile[i], &stStream);
#else
                    // s32Ret = SAMPLE_RTSP_VENC_SaveStream_PhyAddr(pFile[i], &stStreamBufInfo[i], &stStream);
#endif
                    // if (s32Ret != HI_SUCCESS) {
                    //     free(stStream.pstPack);
                    //     stStream.pstPack = NULL;
                    //     SAMPLE_PRT("save stream failed!\n");
                    //     break;
                    // }
#endif
                    // printf("[DBG] start HI_MPI_VENC_ReleaseStream\r\n");
                    s32Ret = HI_MPI_VENC_ReleaseStream(VencChn, &stStream);
                    if (s32Ret != HI_SUCCESS) {
                        SAMPLE_PRT("HI_MPI_VENC_ReleaseStream failed!\n");
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    // else printf("[DBG] HI_MPI_VENC_ReleaseStream success!\r\n");

                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    u32PictureCnt[i]++;
                    // if (PT_JPEG == enPayLoadType[i]) {
                    //     (HI_VOID)fclose(pFile[i]);
                    // }
                }
#ifndef CONFIG_USER_SPACE
            }
        }
#endif
    }

#ifndef OHOS_CONFIG_WITHOUT_SAVE_STREAM
    // for (i = 0; i < s32ChnTotal; i++) {
    //     if (PT_JPEG != enPayLoadType[i]) {
    //         (HI_VOID)fclose(pFile[i]);
    //     }
    // }
#endif
    return NULL;
}

HI_S32 SAMPLE_RTSP_VENC_StartGetStream(VENC_CHN VeChn[], HI_S32 s32Cnt)
{
    HI_S32 i;

    if (VeChn == NULL) {
        SAMPLE_PRT("null pointer\n");
        return HI_FAILURE;
    }
    gs_stPara.bThreadStart = HI_TRUE;
    /* ori
    gs_stPara.s32Cnt = s32Cnt;
    for (i = 0; i < s32Cnt && i < VENC_MAX_CHN_NUM; i++) {
        gs_stPara.VeChn[i] = VeChn[i];
    }
    */
    gs_stPara.s32Cnt = 2;
    gs_stPara.VeChn[0] = VeChn[1];
    gs_stPara.VeChn[1] = VeChn[1];

    return pthread_create(&gs_VencPid, 0, SAMPLE_RTSP_VENC_GetVencStreamProc, (HI_VOID *)&gs_stPara);
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
