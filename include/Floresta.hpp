#pragma once
#include <vector>
#include <string>

// Simula a propagacao de incendio e movimento de um animal
class Floresta {
public:
    // Construtor: inicializa a floresta com matriz e posicoes iniciais
    Floresta(int n, int m, int startRow, int startCol, const std::vector<std::vector<int>>& initialMatrix);

    // Executa uma iteracao completa (movimento do animal + propagacao do fogo)
    void executarIteracao(int iteracao);

    // Verifica se a simulacao terminou
    bool terminou() const;

    // Retorna lista de alteracoes na ultima iteracao
    std::vector<std::string> getLogs() const;

    // Retorna a matriz atual do estado da floresta
    std::vector<std::vector<int>> getMatriz() const;

    // Metodos para estatisticas do animal
    int getPassosAnimal() const;      
    int getAguaEncontrada() const;   
    bool estaAnimalVivo() const;      
    int getIteracaoPreso() const;     
    int getAnimalX() const { return animal.x; }
    int getAnimalY() const { return animal.y; }
private:
    int linhas, colunas;                     
    std::vector<std::vector<int>> matriz;     
    std::vector<std::vector<int>> matrizProxima; 
    int linhaFogo, colunaFogo;               
    int iteracaoAtual;                       

    // Dados do animal
    struct Animal {
        int x, y;                
        int passos;              
        int aguaEncontrada;      
        int iteracaoPreso;       
        int tempoEmSeguro;       
    } animal;

    bool animalVivo;            
    std::vector<std::string> logs; 

    // Metodos internos
    void propagarFogo();  
    void moverAnimal();   
};
