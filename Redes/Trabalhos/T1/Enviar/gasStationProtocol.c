#include "gasStationProtocol.h"
#include <stdlib.h>
#include <time.h>

// Simula um erro no arquivo 
char simulateError(){
    return ((float)rand() / RAND_MAX) > 0.5;
}