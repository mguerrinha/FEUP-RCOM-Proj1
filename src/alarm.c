// Alarm example
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "alarm.h"

int alarmEnabled = FALSE;
int alarmCount = 0;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

void setSignal() {
    (void)signal(SIGALRM, alarmHandler);
}

void setAlarm(int t)
{
    if (alarmEnabled == FALSE)
    {
        alarm(t); // Set alarm to be triggered in 3s
        alarmEnabled = TRUE;
    }
}
