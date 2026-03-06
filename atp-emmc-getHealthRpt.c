#include <linux/types.h>
#include <linux/mmc/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


#include "mmc.h"

// Helper to define MMC command flags based on linux/mmc/ioctl.h
#define MMC_CMD_ADTC (1 << 5) // Addressable Data Transfer Command


#define CMD8                            0x08
#define CMD17                           0x11
#define MMC_CMD_VENDOR_RESVD_1          60
#define MMC_VENDOR_SM_ARG1_SMI          0x534D4900
#define MMC_VENDOR_SM_ARG2_HRPT         0x48525055

#define MMC_CMD_FLAGS_R1B_AC       (MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC)

void hexdump(void *pAddressIn, long  lSize)
{
 char szBuf[100];
 long lIndent = 1;
 long lOutLen, lIndex, lIndex2, lOutLen2;
 long lRelPos;
 struct { char *pData; unsigned long lSize; } buf;
 unsigned char *pTmp,ucTmp;
 unsigned char *pAddress = (unsigned char *)pAddressIn;

   buf.pData   = (char *)pAddress;
   buf.lSize   = lSize;

   while (buf.lSize > 0)
   {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
          lOutLen = 16;

      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
                     "                      "
                     "    %08lX", pTmp-pAddress);
      lOutLen2 = lOutLen;

      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
         )
      {
         ucTmp = *pTmp++;

         sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
         if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
         szBuf[lIndex2] = ucTmp;

         if (!(++lRelPos & 3))     // extra blank after 4 bytes
         {  lIndex++; szBuf[lIndex+2] = ' '; }
      }

      if (!(lRelPos & 3)) lIndex--;

      szBuf[lIndex  ]   = '<';
      szBuf[lIndex+1]   = ' ';

      printf("%s\n", szBuf);

      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
   }
}


int main(int argc, char **argv ) { 

        __u8  cmd17_data[512];
        __u8  cmd8_data[512];


        if (argc != 2) {
                printf( "need a device block to run.\nUsage:  ioctl  < mmc block device>\n");
                exit(1);
        }

        char *device;
        device = argv[1];


    int fd = open( device, O_RDWR); // Change to your device
    if (fd < 0) {
        printf( "Failed to open %s", device);
        return 1;
    } else {
        printf( "Device %s opened\n", device);
    }

    int num_of_cmds = 4;
    // Allocate memory for MULTI_CMD structure
    struct mmc_ioc_multi_cmd *multi_cmd = calloc(1, sizeof(struct mmc_ioc_multi_cmd) + num_of_cmds * sizeof(struct mmc_ioc_cmd));
    
    // Configure one command in the array
    multi_cmd->num_of_cmds     = num_of_cmds;
    struct mmc_ioc_cmd *cmd17  = &multi_cmd->cmds[0];
    struct mmc_ioc_cmd *cmd60a = &multi_cmd->cmds[1];
    struct mmc_ioc_cmd *cmd60b = &multi_cmd->cmds[2];
    struct mmc_ioc_cmd *cmd8   = &multi_cmd->cmds[3];
    
    // SEND_EXT_CSD (CMD17)
    cmd17->opcode     = 17;
    cmd17->arg        = 0;
    cmd17->flags      = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
    cmd17->blksz      = 512;
    cmd17->blocks     = 1;
    mmc_ioc_cmd_set_data(multi_cmd->cmds[0], cmd17_data);
    //     int write_flag;
    //     int is_acmd;
    //     __u32 opcode;
    //     __u32 arg;
    //     __u32 response[4];
    //     unsigned int flags;
    //     unsigned int blksz;
    //     unsigned int blocks;
    //     unsigned int postsleep_min_us;
    //     unsigned int postsleep_max_us;
    //     unsigned int data_timeout_ns;
    //     unsigned int cmd_timeout_ms;
    //     __u32 __pad;
    //     __u64 data_ptr;
    
    
    // SEND_EXT_CSD (CMD60)
    cmd60a->opcode   = 60;
    cmd60a->arg      = MMC_VENDOR_SM_ARG1_SMI;
    cmd60a->flags    = MMC_CMD_FLAGS_R1B_AC;
    cmd60a->blksz    = 0;
    cmd60a->blocks   = 0;
    cmd60a->cmd_timeout_ms = 1 * 1000;
    
    // SEND_EXT_CSD (CMD60)
    cmd60b->opcode   = 60;
    cmd60b->arg      = MMC_VENDOR_SM_ARG2_HRPT;
    cmd60b->flags    = MMC_CMD_FLAGS_R1B_AC;
    cmd60b->blksz    = 0;
    cmd60b->blocks   = 0;
    cmd60b->cmd_timeout_ms = 1 * 1000;
    
    // SEND_EXT_CSD (CMD8)
    cmd8->opcode   = 8;
    cmd8->arg      = 0;
    cmd8->flags    = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
    cmd8->blksz    = 512;
    cmd8->blocks   = 1;
    mmc_ioc_cmd_set_data(multi_cmd->cmds[3], cmd8_data);
    

    // Call ioctl
    int ret = ioctl(fd, MMC_IOC_MULTI_CMD, multi_cmd);
    if (ret) {
        perror("ioctl");
    } else {
        printf("Successfully read EXT_CSD.\n");
        // Access data in ext_csd buffer here
        printf("CMD17 results of %s\n", device);
        hexdump( cmd17_data, 512);
        printf("CMD8 results of %s\n", device);
        hexdump( cmd8_data, 512);
        printf("\n\n");
    }

    free(multi_cmd);
    close(fd);


    return ret;
}

