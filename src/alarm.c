#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "alarm.h"

volatile int alarmEnabled = FALSE;
int alarmCount = 0;
int nRetransmissionsTotal;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
    nRetransmissionsTotal++;

    printf("Alarm #%d\n", alarmCount);
}

void setSignal() {
    (void)signal(SIGALRM, alarmHandler);
}

// Set alarm to be triggered in t secons
void setAlarm(int t)
{
    if (alarmEnabled == FALSE)
    {
        alarm(t); 
        alarmEnabled = TRUE;
    }
}
