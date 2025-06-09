#ifndef GAS_STATION_PROTOCOL_H
#define GAS_STATION_PROTOCOL_H

#define MAX_BUFFER_LEN 128

typedef struct{
    char messageType, error;
    double x, y;
    int id, fuelType, price_radius;
}RequestMessage;

typedef struct{
    char messageType;
    int id;    
}ACKNAK;

typedef struct {
    int id;                 
    int minPrice;       
    double x;        
    double y;
}ResponseMessage;

char simulateError();

#endif
