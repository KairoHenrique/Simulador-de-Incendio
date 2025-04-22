#include <iostream>
#include <fstream>
#include <random>
#include <vector>

using namespace std;

int main() {
    const int N = 100;
    const int M = 100;
    const int fire_row = 50;
    const int fire_col = 500;

    // Configuracao da distribuicao de probabilidades
    vector<int> values{1, 4, 0, 3};
    vector<double> weights{0.70, 0.15, 0.10, 0.05};

    // Configurar gerador de numeros aleatorios
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> dist(weights.begin(), weights.end());

    // Criar e abrir arquivo
    ofstream arquivo("input.dat");
    if(!arquivo.is_open()) {
        cerr << "Erro ao criar arquivo!" << endl;
        return 1;
    }

    // Escrever cabecalho
    arquivo << N << " " << M << " " << fire_row << " " << fire_col << endl;

    // Gerar matriz
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < M; ++j) {
            // Garantir posicao do fogo
            if(i == fire_row && j == fire_col) {
                arquivo << "2";
            } else {
                arquivo << values[dist(gen)];
            }

            // Adicionar espaco entre os numeros
            if(j != M-1) arquivo << " ";
        }
        arquivo << endl;
    }

    arquivo.close();
    cout << "Arquivo input.dat gerado com sucesso!" << endl;
    return 0;
}
