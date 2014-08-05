/*
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2011 Synaptics, Inc.

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to use,
   copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
#include <linux/slab.h>

#include "RefCode.h"
#include "RefCode_PDTScan.h"

#ifdef _F54_TEST_
unsigned char F54_RxOpenReport(void)
{
   unsigned char* ImageBuffer;//[CFG_F54_TXCOUNT*CFG_F54_RXCOUNT*2];
   short** ImageArray = NULL;//[CFG_F54_RXCOUNT][CFG_F54_RXCOUNT];
   //char Result[CFG_F54_RXCOUNT][CFG_F54_RXCOUNT];
   int Result=0;

   short OthersLowerLimit = -100;
   short OthersUpperLimit = 100;

   int i, j, k;
   int length;

   unsigned char command;

   ImageBuffer = kzalloc(CFG_F54_TXCOUNT*CFG_F54_RXCOUNT*2, GFP_KERNEL);
   for (i = 0; i < CFG_F54_RXCOUNT; i++)
   {
       ImageArray[i]  = (short*)kzalloc(CFG_F54_RXCOUNT*2, GFP_KERNEL);
       memset(ImageArray[i], 0x0, CFG_F54_RXCOUNT*2);
   }

   printk("\nBin #: 8		Name: Receiver Open Test\n");
   printk("\n\t");
   for (j = 0; j < numberOfRx; j++) printk("R%d\t", j);
   printk("\n");

   length =  numberOfRx * numberOfTx*2;

   // Set report mode
   command = 0x0E;
   writeRMI(F54_Data_Base, &command, 1);

   // Disable CBC
   command = 0x00;
   writeRMI(F54_CBCSettings, &command, 1);

   //NoCDM4
   command = 0x01;
   writeRMI(NoiseMitigation, &command, 1);

   // Force update
   command = 0x04;
   writeRMI(F54_Command_Base, &command, 1);

   do {
		delayMS(1); //wait 1ms
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   command = 0x02;
   writeRMI(F54_Command_Base, &command, 1);

   do {
		delayMS(1); //wait 1ms
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //  command = 0x00;
   //	writeRMI(0x0113, &command, 1);

   command = 0x00;
   writeRMI(F54_Data_LowIndex, &command, 1);
   writeRMI(F54_Data_HighIndex, &command, 1);

   // Set the GetReport bit
   command = 0x01;
   writeRMI(F54_Command_Base, &command, 1);

   // Wait until the command is completed
   do {
		delayMS(1); //wait 1ms
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   readRMI(F54_Data_Buffer, &ImageBuffer[0], length);

   k = 0;
   for (i = 0; i < numberOfTx; i++)
   {
       for (j = 0; j < numberOfRx; j++)
	   {
			ImageArray[i][j] = (ImageBuffer[k] | (ImageBuffer[k+1] << 8));
			k = k + 2;
	   }
   }

   // Set report mode
   length = numberOfRx* (numberOfRx-numberOfTx) * 2;
   command = 0x12;
   writeRMI(F54_Data_Base, &command, 1);

   command = 0x00;
   writeRMI(F54_Data_LowIndex, &command, 1);
   writeRMI(F54_Data_HighIndex, &command, 1);

   // Set the GetReport bit to run Tx-to-Tx
   command = 0x01;
   writeRMI(F54_Command_Base, &command, 1);

   // Wait until the command is completed
   do {
		delayMS(1); //wait 1ms
		readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   readRMI(F54_Data_Buffer, &ImageBuffer[0], length);

   k = 0;
   for (i = 0; i < (numberOfRx-numberOfTx); i++)
   {
       for (j = 0; j < numberOfRx; j++)
	   {
			ImageArray[numberOfTx+i][j] = ImageBuffer[k] | (ImageBuffer[k+1] << 8);
			k = k + 2;
	   }
   }

   /*
   // Check against test limits
   printk("\nRxToRx Short Test Result :\n");
	for (i = 0; i < numberOfRx; i++)
	{
		for (j = 0; j < numberOfRx; j++)
		{
			if (i == j)
			{
				if((ImageArray[i][j] <= DiagonalUpperLimit) && (ImageArray[i][j] >= DiagonalUpperLimit))
					Result[i][j] = 'P'; //Pass
				else
					Result[i][j] = 'F'; //Fail
				//printk("%3d", ImageArray[i][j]);
			}
			else
			{
				if(ImageArray[i][j] <= OthersUpperLimit)
					Result[i][j] = 'P'; //Fail
				else
					Result[i][j] = 'F'; //Fail
			}
			printk("%4d", ImageArray[i][j]);
		}
		printk("\n");
	}
	printk("\n");
	*/

	for (i = 0; i < numberOfRx; i++)
	{
		printk("R%d\t", i);
		for (j = 0; j < numberOfRx; j++)
		{
			if((ImageArray[i][j] <= OthersUpperLimit) && (ImageArray[i][j] >= OthersLowerLimit))
			{
				Result++; //Pass
				printk("%d\t", ImageArray[i][j]);
			}
			else
			{
				printk("%d(*)\t", ImageArray[i][j]);
			}
		}
		printk("\n");
	}

   // Set the Force Cal
   command = 0x02;
   writeRMI(F54_Command_Base, &command, 1);

   do {
		delayMS(1); //wait 1ms
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //enable all the interrupts
//   SetPage(0x00);
   //Reset
   command= 0x01;
   writeRMI(F01_Cmd_Base, &command, 1);
   delayMS(200);
   readRMI(F01_Data_Base+1, &command, 1); //Read Interrupt status register to Interrupt line goes to high

   kfree(ImageBuffer);
   for (i = 0; i < CFG_F54_RXCOUNT; i++)
       kfree(ImageArray[i]);

   //printk("Result = %d, Rx*Rx= %d\n", Result, numberOfRx * numberOfRx);
   if(Result == numberOfRx * numberOfRx)
	{
		printk("Test Result: Pass\n");
		return 1; //Pass
	}
   else
	{
		printk("Test Result: Fail\n");
		return 0; //Fail
	}
}
#endif

