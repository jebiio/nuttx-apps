/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "system/readline.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

bool gApgLoop = false;

void dwm_api(int fd, char *cmd)
{
    int ret;
    char str[128];

    if (strcmp(cmd, "double_enter") == 0)
    {
        str[0] = 0x0d;
        str[1] = 0x0d;
        ret = write(fd, str, 2);
        if (ret < 0)
        {
            printf("error :");
        }
    }
    else
    {
        strcpy(str, cmd);
        strcat(str, "\r");
        ret = write(fd, str, strlen(str));
        if (ret < 0)
        {
            printf("error :");
        }
    }
}

static int read_thread(int argc, FAR char *argv[])
{
    int fd, ret = 0;

    fd = open("/dev/ttyS1", O_RDWR);
    if (fd < 0)
    {
        printf("Error UART");
        return 0;
    }

    char ch;
    char buffer[128];
    int index = 0;

    while (1)
    {
        ret = read(fd, &ch, sizeof(ch));
        if (ret > 0)
        {
            if (ch == '\n')
            {
                buffer[index++] = '\0';
                if(gApgLoop && strstr(buffer, "apg:")!=NULL)
                {
                    printf("%s\n", buffer);
                }
                else
                {
                    buffer[index] = '\r';
                    printf("%s\n", buffer);
                }

                index = 0;
            }
            else
            {
                buffer[index] = ch;
                index++;
            }
        }
    }

    fflush(stdout);
    close(fd);
    return 0;
}

static int apg_thread(int argc, FAR char *argv[])
{
    int fd, ret = 0;
    char buffer[4];

    fd = open("/dev/ttyS1", O_RDWR);
    if (fd < 0)
    {
        printf("Error UART");
        return 0;
    }

    buffer[0] = 'a';
    buffer[1] = 'p';
    buffer[2] = 'g';
    buffer[3] = '\r';

    while (1)
    {
        ret = write(fd, buffer, 4);
        if (ret < 0)
        {
            printf("write error\n");
        }
        sleep(1);
    }
    fflush(stdout);
    close(fd);
    return 0;
}

int main(int argc, FAR char *argv[])
{
    int fd_console, fd_uwb, ret = 0;
    char buffer[1024];
    int index = 0;
    char ch;
    int apt_task_id;

    fd_console = open("/dev/ttyS0", O_RDWR);
    if (fd_console < 0)
    {
        printf("uwb_main: ERROR: Failed to open /dev/ttyS0\n");
        fflush(stdout);
        return 0;
    }

    fd_uwb = open("/dev/ttyS1", O_RDWR);
    if (fd_uwb < 0)
    {
        printf("uwb_main: ERROR: Failed to open /dev/ttyS1\n");
        fflush(stdout);
        return 0;
    }

    ret = task_create("read_thread", CONFIG_EXAMPLES_UWB_PRIORITY,
                      1024, read_thread,
                      NULL);
    if (ret < 0)
    {
        int errcode = errno;
        printf("uwb_main: ERROR: Failed to start read_thread: %d\n",
               errcode);
        return EXIT_FAILURE;
    }

    sleep(0.5);
    dwm_api(fd_uwb, "double_enter");

    while (1)
    {
        ret = read(fd_console, &ch, 1);
        if (ret < 1)
        {
            ch = '!';
        }
        else if ((ch < 0x20 || ch > 0x7e) && ch != '\n')
        {
            ch = '.';
        }

        if (ch == '\n')
        {
            buffer[index++] = '\0';
            if (strcmp(buffer, "start_apg_loop") == 0)
            {
                ret = task_create("apg_loop", CONFIG_EXAMPLES_UWB_PRIORITY,
                                  512, apg_thread,
                                  NULL);
                if (ret < 0)
                {
                    int errcode = errno;
                    printf("uwb_main: ERROR: Failed to start apg_thread: %d\n",
                           errcode);
                    return EXIT_FAILURE;
                }
                else
                {
                    apt_task_id = ret;
                    printf("uwb_main: apg_thread started: %d\n",
                           apt_task_id);
                    gApgLoop = true;
                }
                index = 0;
                continue;
            }
            else
            {
                buffer[index++] = '\r';
                ret = write(fd_uwb, buffer, index);
                if (ret < 0)
                {
                    printf("write error\n");
                }
                index = 0;
            }
        }
        else
        {
            buffer[index++] = ch; // 버퍼에 문자 저장
        }
    }

    fflush(stdout);
    close(fd_console);
    return 0;
}