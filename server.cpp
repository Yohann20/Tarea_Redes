#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>

using namespace std;

const int ROWS = 6;
const int COLS = 7;

// Estructura para representar un movimiento
struct Move {
    int column;
};

// Clase para el servidor
class Server {
private:
    int serverSocket;
    vector<int> clientSockets;
    char board[ROWS][COLS];
    int currentPlayer;
    int port;

public:
    Server(int _port) : port(_port) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error al crear el socket del servidor" << endl;
            exit(1);
        }

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            cerr << "Error al hacer el bind" << endl;
            exit(1);
        }

        if (listen(serverSocket, 5) == -1) {
            cerr << "Error al escuchar en el puerto" << endl;
            exit(1);
        }

        memset(board, ' ', sizeof(board));
        currentPlayer = 1;
    }

    // Función para manejar las conexiones entrantes de los clientes
    void handleConnections() {
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
                cerr << "Error al aceptar la conexión del cliente" << endl;
                continue;
            }
            cout << "Cliente conectado" << endl;
            clientSockets.push_back(clientSocket);

            // Crear un hilo para manejar las acciones del cliente
            pthread_t thread;
            pthread_create(&thread, nullptr, handleClient, &clientSocket);
            pthread_detach(thread);
        }
    }

    // Función para manejar las acciones de un cliente específico
    static void *handleClient(void *arg) {
        int clientSocket = *((int *)arg);
        Server server(0); // Creamos una instancia temporal del servidor para manejar al cliente
        server.playGame(clientSocket);
        close(clientSocket);
        cout << "Cliente desconectado" << endl;
        return nullptr;
    }

    // Función para jugar el juego con un cliente específico
    void playGame(int clientSocket) {
        int player = clientSockets.size(); // ID del jugador
        srand(time(nullptr)); // Semilla para la selección aleatoria de la columna inicial
        int startingPlayer = rand() % 2 + 1; // Selección aleatoria del jugador que inicia

        // Enviar mensaje al cliente sobre quién comienza
        send(clientSocket, &startingPlayer, sizeof(startingPlayer), 0);

        // Inicializar el juego
        currentPlayer = startingPlayer;

        while (!isGameOver()) {
            // Turno del cliente
            if (currentPlayer == 1) {
                // Enviar el tablero actual al cliente
                send(clientSocket, &board, sizeof(board), 0);

                // Esperar movimiento del cliente
                Move move;
                recv(clientSocket, &move, sizeof(move), 0);

                // Validar y aplicar el movimiento
                if (isValidMove(move)) {
                    applyMove(move);
                    // Verificar si hay un ganador
                    if (checkWinner()) {
                        // Enviar mensaje de fin del juego con el estado "ganador"
                        int winner = currentPlayer;
                        send(clientSocket, &winner, sizeof(winner), 0);
                        break;
                    } else if (isBoardFull()) {
                        // Enviar mensaje de fin del juego con el estado "empate"
                        int draw = 0;
                        send(clientSocket, &draw, sizeof(draw), 0);
                        break;
                    } else {
                        // Cambiar el turno
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    }
                } else {
                    // Enviar mensaje de movimiento inválido al cliente
                    int invalidMove = -1;
                    send(clientSocket, &invalidMove, sizeof(invalidMove), 0);
                }
            } else {
                // Turno del servidor
                // Seleccionar aleatoriamente una columna válida
                int randomColumn;
                do {
                    randomColumn = rand() % COLS;
                } while (!isValidMove({randomColumn}));

                // Aplicar el movimiento del servidor
                applyMove({randomColumn});

                // Verificar si hay un ganador
                if (checkWinner()) {
                    // Enviar mensaje de fin del juego con el estado "ganador"
                    int winner = currentPlayer;
                    send(clientSocket, &winner, sizeof(winner), 0);
                    break;
                } else if (isBoardFull()) {
                    // Enviar mensaje de fin del juego con el estado "empate"
                    int draw = 0;
                    send(clientSocket, &draw, sizeof(draw), 0);
                    break;
                } else {
                    // Cambiar el turno
                    currentPlayer = (currentPlayer == 1) ? 2 : 1;
                }
            }
        }
    }

    // Funciones para la lógica del juego
    bool isValidMove(Move move) {
        return (move.column >= 0 && move.column < COLS && board[0][move.column] == ' ');
    }

    void applyMove(Move move) {
        int row = ROWS - 1;
        while (board[row][move.column] != ' ' && row >= 0) {
            row--;
        }
        if (row >= 0) {
            board[row][move.column] = (currentPlayer == 1) ? 'X' : 'O';
        }
    }

    bool isGameOver() {
        return checkWinner() || isBoardFull();
    }

    bool checkWinner() {
    // Verificar horizontalmente
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS - 3; ++col) {
            if (board[row][col] != ' ' &&
                board[row][col] == board[row][col + 1] &&
                board[row][col] == board[row][col + 2] &&
                board[row][col] == board[row][col + 3]) {
                return true;
            }
        }
    }

    // Verificar verticalmente
    for (int col = 0; col < COLS; ++col) {
        for (int row = 0; row < ROWS - 3; ++row) {
            if (board[row][col] != ' ' &&
                board[row][col] == board[row + 1][col] &&
                board[row][col] == board[row + 2][col] &&
                board[row][col] == board[row + 3][col]) {
                return true;
            }
        }
    }

    // Verificar diagonalmente (hacia abajo y hacia la derecha)
    for (int row = 0; row < ROWS - 3; ++row) {
        for (int col = 0; col < COLS - 3; ++col) {
            if (board[row][col] != ' ' &&
                board[row][col] == board[row + 1][col + 1] &&
                board[row][col] == board[row + 2][col + 2] &&
                board[row][col] == board[row + 3][col + 3]) {
                return true;
            }
        }
    }

    // Verificar diagonalmente (hacia arriba y hacia la derecha)
    for (int row = 3; row < ROWS; ++row) {
        for (int col = 0; col < COLS - 3; ++col) {
            if (board[row][col] != ' ' &&
                board[row][col] == board[row - 1][col + 1] &&
                board[row][col] == board[row - 2][col + 2] &&
                board[row][col] == board[row - 3][col + 3]) {
                return true;
            }
        }
    }

    return false;
}

    bool isBoardFull() {
        for (int col = 0; col < COLS; col++) {
            if (board[0][col] == ' ') {
                return false;
            }
        }
        return true;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <puerto>" << endl;
        return 1;
    }

    int port = atoi(argv[1]); // Puerto en el que escuchará el servidor
    Server server(port);
    server.handleConnections();
    return 0;
}
