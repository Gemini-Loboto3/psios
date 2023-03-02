#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "common.h"

// globals for display envoirment
volatile GsOT WorldOrderingTable[2];
GsOT_TAG zSortTable[2][1<<OT_LENGTH];
volatile PACKET GpuOutputPacket[2][PACKETMAX2];
volatile int outputBufferIndex;
