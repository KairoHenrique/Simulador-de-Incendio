#include <iostream>
#include <fstream>
#include "Floresta.hpp"
#include "Config.hpp"
#include "LerArquivo.hpp"

int main() {
    // Abre arquivo de entrada
    std::ifstream entrada("data/input.dat");
    std::ofstream saida("data/output.dat");

    // Le dimensoes e posicao inicial do fogo
    int n, m, fr, fc;
    entrada >> n >> m >> fr >> fc;

    // Le matriz do arquivo
    auto matriz = lerArquivo(entrada, n, m);

    // Inicia simulacao
    Floresta floresta(n, m, fr, fc, matriz);
    int iteracao = 1;

    // Loop de simulacao
    while (!floresta.terminou() && iteracao <= Config::MaxIteracoes) {
        floresta.executarIteracao(iteracao);

        // Exibe no console
        if (Config::exibirNoConsole) {
            std::cout << "\n=== Iteracao " << iteracao << " ===" << std::endl;
            for (const auto& log : floresta.getLogs()) {
                std::cout << log << std::endl;
            }
            
            // Exibe matriz com posicao do animal
            size_t linha_atual = 0;
            for (const auto& linha : floresta.getMatriz()) {
                for (size_t coluna_atual = 0; coluna_atual < linha.size(); coluna_atual++) {
                    if (linha_atual == static_cast<size_t>(floresta.getAnimalX()) && 
                        coluna_atual == static_cast<size_t>(floresta.getAnimalY())) {
                        std::cout << "A ";
                    } else {
                        std::cout << linha[coluna_atual] << " ";
                    }
                }
                std::cout << std::endl;
                linha_atual++;
            }
        }

        // Escreve no arquivo de saida
        for (const auto& log : floresta.getLogs()) {
            saida << log << std::endl;
        }
        
        // Escreve matriz com posicao do animal
        saida << "Iteracao " << iteracao << ":" << std::endl;
        size_t linha_atual = 0;
        for (const auto& linha : floresta.getMatriz()) {
            for (size_t coluna_atual = 0; coluna_atual < linha.size(); coluna_atual++) {
                if (linha_atual == static_cast<size_t>(floresta.getAnimalX()) && 
                    coluna_atual == static_cast<size_t>(floresta.getAnimalY())) {
                    saida << "A ";
                } else {
                    saida << linha[coluna_atual] << " ";
                }
            }
            saida << std::endl;
            linha_atual++;
        }

        iteracao++;
    }

    // Exibe estatisticas
    saida << "\n=== Estatisticas do Animal ===" << std::endl;
    saida << "Passos: " << floresta.getPassosAnimal() << std::endl;
    saida << "Agua encontrada: " << floresta.getAguaEncontrada() << std::endl;
    saida << "Sobreviveu: " << (floresta.estaAnimalVivo() ? "Sim" : "Nao") << std::endl;
    if (!floresta.estaAnimalVivo()) {
        saida << "Iteracao preso: " << floresta.getIteracaoPreso() << std::endl;
    }

    // Fecha arquivos
    entrada.close();
    saida.close();
    return 0;
}