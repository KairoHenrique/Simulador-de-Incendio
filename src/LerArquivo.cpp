#include "LerArquivo.hpp"


std::vector<std::vector<int>> lerArquivo(std::ifstream& arquivo, int linhas, int colunas) {
    std::vector<std::vector<int>> matriz(linhas, std::vector<int>(colunas));
    for (int i = 0; i < linhas; ++i) {
        for (int j = 0; j < colunas; ++j) {
            arquivo >> matriz[i][j];
        }
    }
    return matriz;
}