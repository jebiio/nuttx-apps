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

#include "system/readline.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

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
    char buffer;

    fd = open("/dev/ttyS1", O_RDWR);
    if (fd < 0)
    {
        printf("Error UART");
        return 0;
    }

    while (1)
    {
        ret = read(fd, &buffer, sizeof(buffer)); // It return only a char
        if (ret != 0)
        {
            printf("%c", buffer);
        }
    }
    fflush(stdout);
    close(fd);
    return 0;
}

int main(int argc, FAR char *argv[])
{
    int fd, ret, len = 0;
    char str[128];

    fd = open("/dev/ttyS1", O_RDWR);
    if (fd < 0)
    {
        printf("aaa_main: ERROR: Failed to open /dev/ttyS1\n");
        fflush(stdout);
        return 0;
    }

    dwm_api(fd, "double_enter");

    ret = task_create("read_thread", CONFIG_EXAMPLES_AAA_PRIORITY,
                      CONFIG_EXAMPLES_AAA_STACKSIZE, read_thread,
                      NULL);
    if (ret < 0)
    {
        int errcode = errno;
        printf("aaa_main: ERROR: Failed to start read_main: %d\n",
               errcode);
        return EXIT_FAILURE;
    }

    while (1)
    {
        char * ch;
        ch = readline(NULL);
        if (strncmp(ch, "double_enter", 12) == 0)
        {
            dwm_api(fd, "double_enter");
        }
        else
        {
            len = strlen(ch);
            ret = write(fd, ch, len);
        }
    }
    fflush(stdout);
    close(fd);
    return 0;
}