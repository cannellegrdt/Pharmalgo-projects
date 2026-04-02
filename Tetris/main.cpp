#include <wiringPi.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include "../Lib_Croix/CroixPharma.h"

void *cmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

int my_strlen(char const *str) {
    int i = 0;
    while (str[i]) i++;
    return i;
}

void send_to_x(CroixPharma &croix, char *data) {
    uint8_t bitmap[24][24];
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 24; x++) {
            bitmap[y][x] = (data[y * 24 + x] == '0') ? 0 : 1;
        }
    }
    croix.writeBitmap(bitmap);
}

void set_data(char *str) {
    for (int i = 0; i < 576; i++) str[i] = '0';
}

void des_carrer(char *data, int nb, char c, int *carrer) {
    data[nb] = c;
    data[nb + 1] = c;
    data[nb + 24] = c;
    data[nb + 25] = c;
}

int can_move(int nb, int add, char *data) {
    if (nb + add >= 0 && nb + add <= 576 && (nb + add) % 24 > 7 && (nb + add) % 24 < 16 && data[nb + add] == '0')
        return 1;
    return 0;
}

int move(int nb, int add, char *data, int *carrer) {
    int move_can = 1;
    for (int i = 0; i < 4; i++) {
        if (can_move(nb, add + carrer[i], data) == 0)
            move_can = 0;
    }
    if (move_can == 1) return nb + add;
    return nb;
}

void del_ligne(char *data, int i) {
    for (; i >= 0; i -= 24) {
        for (int j = 8; j < 16; j++) {
            data[i + j + 24] = data[i + j];
            data[i + j] = '0';
        }
    }
}

void detect_complet_ligne(char *data) {
    int ligne = 0;
    for (int i = 0; i < 572; i += 24) {
        ligne = 1;
        for (int j = 8; j < 16; j++) {
            if (data[i + j] == '0') ligne = 0;
        }
        if (ligne == 1) {
            i -= 24;
            del_ligne(data, i);
        }
    }
}

int main() {
    if (wiringPiSetupGpio() < 0) return 1;
    CroixPharma croix;
    croix.begin();
    croix.setSide(CroixPharma::BOTH);

    char *data = (char *)cmalloc(sizeof(char) * (576 + 1));
    set_data(data);
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    char c;
    int nb = 12;
    int nb_boucle = 0;
    int carrer[4] = {0, 1, 24, 25};
    int en_bas = 0;

    printf("Tetris \n");

    while (1) {
        des_carrer(data, nb, '0', carrer);
        c = getchar();
        if (c == 'c') break;
        if (c == 'z') nb = move(nb, -24, data, carrer);
        if (c == 'q') nb = move(nb, -1, data, carrer);
        if (c == 's') nb = move(nb, 24, data, carrer);
        if (c == 'd') nb = move(nb, 1, data, carrer);
        if (nb_boucle % 10 == 0) {
            en_bas = move(nb, 24, data, carrer);
            if (en_bas == nb) {
                des_carrer(data, nb, '7', carrer);
                nb = 12;
                detect_complet_ligne(data);
            } else {
                nb = en_bas;
            }
        }

        des_carrer(data, nb, '7', carrer);
        send_to_x(croix, data);
        usleep(30000);
        nb_boucle++;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    free(data);
    return 0;
}
