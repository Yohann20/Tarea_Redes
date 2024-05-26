#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <arpa/inet.h> // 

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
        
    }

    // Función para manejar las conexiones entrantes de los clientes
    void handleConnections() {
        cout << "Esperando conexiones ..." << endl;
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
                cerr << "Error al aceptar la conexión del cliente" << endl;
                continue;
            }
            cout << "Juego nuevo[" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]" << endl;
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
        server.Jugar(clientSocket);
        close(clientSocket);
        cout << "Cliente desconectado" << endl;
        return nullptr;
    }

    // Función para jugar el juego con un cliente específico
    void Jugar(int clientSocket) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        getpeername(clientSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        cout << "Juego [" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]: ";
        srand(time(nullptr)); // Semilla para la selección aleatoria de la columna inicial
        int startingPlayer = rand() % 2 + 1;   // Selección aleatoria del jugador que inicia
        cout << "inicia juego el " << (startingPlayer == 1 ? "cliente" : "servidor") << "." << endl;
        currentPlayer = startingPlayer; 
        if (startingPlayer == 2){ 
            MovimientoServidor(clientSocket); 
        }
        while (!isGameOver()) { 
                // Turno del cliente
                sendBoard(clientSocket);
                sendMessage(clientSocket, "Es tu turno jugador Ingresa el número de columna donde dejaras caer tu ficha (1-7): ");
                
                Move move;
                recv(clientSocket, &move, sizeof(move), 0);

                if (isValidMove(move)) { 
                    currentPlayer = 1; 
                    Movimiento(move, clientSocket);
                    if (VerificarGanador()) { 
                        sendBoard(clientSocket);
                        sendMessage(clientSocket, "¡Has ganado!");
                        cout << "¡El cliente ha ganado!" << endl; 
                        break;
                    } else if (VerificarTablero()) {  
                        sendBoard(clientSocket);
                        sendMessage(clientSocket, "¡Empate!");
                        cout << "¡Empate!" << endl;
                        break;
                    }
                } else { 
                    sendBoard(clientSocket);
                    sendMessage(clientSocket, "¡Movimiento invalido!");
                    cout << "¡Movimiento invalido!" << endl;
                    break;
                } 
                
                // Turno del servidor 
                currentPlayer = 2; 
                MovimientoServidor(clientSocket);
                
                if (VerificarGanador()) {
                    sendBoard(clientSocket); 
                    sendMessage(clientSocket, "¡El servidor a ganado!");
                    cout << "¡El Servidor ha ganado!" << endl;
                    break;
                } else if (VerificarTablero()) { 
                    sendBoard(clientSocket);
                    sendMessage(clientSocket, "¡Empate!");
                    cout << "¡Empate!" << endl;
                    break;
                }     
            
        }  
        close(clientSocket); 
    }  

    void MovimientoServidor(int clientSocket) {
        int randomColumn;
        do {
            randomColumn = rand() % COLS;
        } while (!isValidMove({randomColumn}));

        Movimiento({randomColumn}, clientSocket);
    }  
    void sendBoard(int clientSocket) {
        send(clientSocket, &board, sizeof(board), 0);
    }

    void sendMessage(int clientSocket, const string &message) {
        send(clientSocket, message.c_str(), message.size() + 1, 0); // +1 to include null terminator
    }

    

    

    // Funciones para la lógica del juego
    bool isValidMove(Move move) {
        return (move.column >= 0 && move.column < COLS && board[0][move.column] == ' ');
    } 

    void Movimiento(Move move, int clientSocket) {
        int row = ROWS - 1;
        while (board[row][move.column] != ' ' && row >= 0) {
            row--;
        }
        if (row >= 0) {
            board[row][move.column] = (currentPlayer == 1) ? 'C' : 'S';
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            getpeername(clientSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
            cout << "Juego [" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]: ";
            cout << (currentPlayer == 1 ? "cliente" : "servidor") << " juega columna " << move.column + 1 << "." << endl;
        }
    }

    bool isGameOver() {
        return VerificarGanador() || VerificarTablero();
    }

    bool VerificarGanador() {
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

    bool VerificarTablero() {
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