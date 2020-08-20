#include "Matrix.h"
#include "MatrixServer.h"

Matrix ledMatrix;

short cathodePins[] = { 11, A0, 12, 13, A1, A2, A3, A4, A5 };
short anodePins[]   = { A5, 4, 5, 7, 6, 8, 9, 10 };

void setup() {
    Serial.begin(9600);
    ledMatrix.init(sizeof(cathodePins) / sizeof(short), sizeof(anodePins) / sizeof(short), cathodePins, anodePins);
}

void loop() {
    MatrixServer server("ENGEL_TP_ACCESSPOINT", "engelwifi", 1337);
    server.enterLoop(ledMatrix);
}
