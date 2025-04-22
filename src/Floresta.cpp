#include "Floresta.hpp"
#include "Config.hpp"
#include <algorithm>
#include <sstream>

// Construtor: define estado inicial
Floresta::Floresta(int n, int m, int startRow, int startCol, const std::vector<std::vector<int>>& initialMatrix)
    : linhas(n), colunas(m),
      matriz(initialMatrix), matrizProxima(initialMatrix),
      linhaFogo(startRow), colunaFogo(startCol),
      iteracaoAtual(0), animalVivo(true) {

    // Inicia o fogo na posicao especificada
    matriz[linhaFogo][colunaFogo] = 2;

    // Busca posicao inicial segura para o animal (valor 0)
    bool encontrado = false;
    for (int i = 0; i < linhas && !encontrado; ++i) {
        for (int j = 0; j < colunas && !encontrado; ++j) {
            if (matriz[i][j] == 0) {
                animal.x = i;
                animal.y = j;
                encontrado = true;
            }
        }
    }
    if (!encontrado) animalVivo = false;

    // Inicializa dados do animal
    animal.passos = 0;
    animal.aguaEncontrada = 0;
    animal.iteracaoPreso = -1;
    animal.tempoEmSeguro = 0;
}

// Move o animal priorizando agua > seguro > queimado
void Floresta::moverAnimal() {
    if (!animalVivo) return;

    int x = animal.x;
    int y = animal.y;

    // Verifica tempo em celula segura (max 3 iteracoes)
    if (matriz[x][y] == 0) {
        if (animal.tempoEmSeguro++ < 3) return;
        else animal.tempoEmSeguro = 0;
    }

    // Direcoes possiveis (cima, baixo, esquerda, direita)
    struct Direcao { int dx, dy; };
    std::vector<Direcao> direcoes = {{-1,0}, {1,0}, {0,-1}, {0,1}};
    std::vector<std::pair<int, int>> opcoes;

    // Avalia vizinhos validos
    for (size_t i = 0; i < direcoes.size(); ++i) {
        int nx = x + direcoes[i].dx;
        int ny = y + direcoes[i].dy;
        if (nx < 0 || nx >= linhas || ny < 0 || ny >= colunas) continue;

        int valor = matriz[nx][ny];
        int prioridade;
        if (valor == 4) prioridade = 0;       // Agua (prioridade maxima)
        else if (valor == 0 || valor == 1) prioridade = 1;  // Seguro
        else if (valor == 3) prioridade = 2;  // Queimado
        else continue;                        // Ignora fogo

        opcoes.emplace_back(prioridade, i);
    }

    // Sem opcoes disponiveis, animal preso
    if (opcoes.empty()) {
        animalVivo = false;
        animal.iteracaoPreso = iteracaoAtual;
        return;
    }

    // Seleciona melhor opcao e atualiza posicao
    std::sort(opcoes.begin(), opcoes.end());
    int idx = opcoes[0].second;
    animal.x += direcoes[idx].dx;
    animal.y += direcoes[idx].dy;
    animal.passos++;

    // Trata celula com agua
    if (matriz[animal.x][animal.y] == 4) {
        animal.aguaEncontrada++;
        matriz[animal.x][animal.y] = 0;
        logs.push_back("(" + std::to_string(animal.x) + "," + std::to_string(animal.y) + ") vira 0");

        // Converte vizinhos para arvores saudaveis
        for (const auto& dir : direcoes) {
            int ax = animal.x + dir.dx;
            int ay = animal.y + dir.dy;
            if (ax >= 0 && ax < linhas && ay >= 0 && ay < colunas && matriz[ax][ay] != 1) {
                matriz[ax][ay] = 1;
                logs.push_back("(" + std::to_string(ax) + "," + std::to_string(ay) + ") vira 1");
            }
        }
    }
}

// Propaga fogo conforme direcoes do vento
void Floresta::propagarFogo() {
    matrizProxima = matriz;
    for (int i = 0; i < linhas; ++i) {
        for (int j = 0; j < colunas; ++j) {
            if (matriz[i][j] == 2) {
                matrizProxima[i][j] = 3; // Marca como queimado

                // Direcoes ativas do vento
                std::vector<std::pair<int, int>> direcoes;
                if (Config::Vento[0]) direcoes.emplace_back(-1, 0); // Cima
                if (Config::Vento[1]) direcoes.emplace_back(1, 0);   // Baixo
                if (Config::Vento[2]) direcoes.emplace_back(0, -1);  // Esquerda
                if (Config::Vento[3]) direcoes.emplace_back(0, 1);   // Direita

                // Propaga para vizinhos
                for (const auto& dir : direcoes) {
                    int x = i + dir.first;
                    int y = j + dir.second;
                    if (x >= 0 && x < linhas && y >= 0 && y < colunas && matriz[x][y] == 1) {
                        matrizProxima[x][y] = 2;
                        std::string direcao;
                        if (dir.first == -1) direcao = "acima";
                        else if (dir.first == 1) direcao = "abaixo";
                        else if (dir.second == -1) direcao = "esquerda";
                        else direcao = "direita";
                        logs.push_back("(" + std::to_string(x) + "," + std::to_string(y) + ") vira 2 (" + direcao + ")");
                    }
                }
            }
        }
    }
    matriz = matrizProxima;
}

// Executa uma iteracao completa
void Floresta::executarIteracao(int iteracao) {
    iteracaoAtual = iteracao;
    logs.clear();
    logs.push_back("Animal em: (" + std::to_string(animal.x) + "," + std::to_string(animal.y) + ")");
    moverAnimal();
    propagarFogo();

    // Segunda chance se animal em fogo
    if (matriz[animal.x][animal.y] == 2) {
        moverAnimal();
    }

   
}

// Verifica se a simulacao terminou
bool Floresta::terminou() const {
    bool temFogo = false;
    for (const auto& linha : matriz) {
        for (int celula : linha) {
            if (celula == 2) temFogo = true;
        }
    }
    return !temFogo || !animalVivo || iteracaoAtual >= Config::MaxIteracoes;
}

// ========== GETTERS ========== //
std::vector<std::string> Floresta::getLogs() const { return logs; }
std::vector<std::vector<int>> Floresta::getMatriz() const { return matriz; }
int Floresta::getPassosAnimal() const { return animal.passos; }
int Floresta::getAguaEncontrada() const { return animal.aguaEncontrada; }
bool Floresta::estaAnimalVivo() const { return animalVivo; }
int Floresta::getIteracaoPreso() const { return animal.iteracaoPreso; }
