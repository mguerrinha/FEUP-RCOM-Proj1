// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdio.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // TODO
    /*
    - criar estrutura "connectionParam" do tipo LinkLayer com os valores de 5 parâmetros que recebeu (não passa filename)
    - invoca llopen passando o "connectionParam"

    - se Tx
    -- cria pacote START
    -- invoca llwrite passando START
    -- abre ficheiro "filename" em modo de leitura
    -- enquanto não chega ao fim do ficheiro:
    --- lê segmento de k bytes
    --- cria pacote de dados
    --- invoca llwrite passando pacote de dados
    -- cria pacote END e invoca llwrite

    - se Rx
    -- enquanto não recebe pacote END:
    --- invoca llread

    - invoca llclose
    */

   LinkLayerRole linkLayerRole;

   if (strcmp(role, "tx") == 0) {
       linkLayerRole = LlTx;       
   }
   else if (strcmp(role, "rx") == 0) {
       linkLayerRole = LlRx;
   }
   else {
       printf("Wrong role\n");
   }

   LinkLayer connectionParam;

   strcpy(connectionParam.serialPort, serialPort);
   connectionParam.role = linkLayerRole;
   connectionParam.baudRate = baudRate;
   connectionParam.nRetransmissions = nTries;
   connectionParam.timeout = timeout;

   printf("LLOPEN\n");
   
   llopen(connectionParam);
   
}
