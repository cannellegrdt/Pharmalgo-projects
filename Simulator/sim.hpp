#include <stdint.h>

// états logiques
#define HIGH 1
#define LOW  0

// Pins disponibles pour la croix
#define SORTIE_1  15
#define SORTIE_2  16
#define SORTIE_3  14
#define SORTIE_4  17

// initialise le simulateur
void sim_init();

// Permet de définir la valeur d'un pin
void sim(uint8_t pin, uint8_t value);
