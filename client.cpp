#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <arpa/inet.h>

using namespace std;

const int ROWS = 6;
const int COLS = 7;

// Estructura para representar un movimiento
struct Move {
    int column;
};

// Función para visualizar el tablero
void displayBoard(char board[ROWS][COLS]) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cout << board[row][col] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <dirección IP> <puerto>" << endl;
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error al crear el socket del cliente" << endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Error al conectar con el servidor" << endl;
        return 1;
    }

    cout << "Conectado al servidor" << endl;

    char board[ROWS][COLS];

    // Recibir indicación del servidor sobre quién comienza
    int startingPlayer;
    recv(clientSocket, &startingPlayer, sizeof(startingPlayer), 0);
    cout << "El jugador " << startingPlayer << " comienza el juego." << endl;

    // Inicializar el tablero
    memset(board, ' ', sizeof(board));

    while (true) {
        // Turno del servidor
        if (startingPlayer == 2) {
            // Recibir actualización del tablero desde el servidor
            recv(clientSocket, &board, sizeof(board), 0);
            displayBoard(board);

            // Verificar resultado del juego
            int winner;
            if (recv(clientSocket, &winner, sizeof(winner), MSG_DONTWAIT) > 0) {
                if (winner == 0) {
                    cout << "¡Empate!" << endl;
                } else {
                    cout << "¡El jugador " << winner << " ha ganado!" << endl;
                }
                break;
            }
        } else {
            // Turno del cliente
            // Visualizar el tablero
            displayBoard(board);

            // Esperar movimiento del cliente
            cout << "Es tu turno (jugador 1). Ingresa el número de columna donde dejar caer tu ficha (0-6): ";
            int column;
            cin >> column;

            // Enviar movimiento al servidor
            Move move;
            move.column = column;
            send(clientSocket, &move, sizeof(move), 0);

            // Recibir actualización del tablero desde el servidor
            recv(clientSocket, &board, sizeof(board), 0);

            // Verificar resultado del juego
            int winner;
            if (recv(clientSocket, &winner, sizeof(winner), MSG_DONTWAIT) > 0) {
                if (winner == 0) {
                    cout << "¡Empate!" << endl;
                } else {
                    cout << "¡El jugador " << winner << " ha ganado!" << endl;
                }
                break;
            }
        }
    }

    close(clientSocket);
    return 0;
}