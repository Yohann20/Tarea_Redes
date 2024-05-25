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
    cout << "TABLERO" << endl;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cout << board[row][col] << " ";
        }
        cout << endl;
    }
    cout << "-------------" << endl;
    cout << "1 2 3 4 5 6 7" << endl;
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
    //cout << "El jugador " << startingPlayer << " comienza el juego." << endl; 
    //int currentPlayer = startingPlayer; 
    

    // Inicializar el tablero
    memset(board, ' ', sizeof(board));

    while (true) { 
        
        recv(clientSocket, &board, sizeof(board), 0); // Recibir el tablero actualizado

        displayBoard(board);

        // Turno del cliente
        cout << "Es tu turno (jugador 1). Ingresa el número de columna donde dejar caer tu ficha (1-7): ";
        int column;
        cin >> column;

        Move move;
        move.column = column - 1;
        send(clientSocket, &move, sizeof(move), 0);
            
        
            
    }

        close(clientSocket);
        return 0;
} 
