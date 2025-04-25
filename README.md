<h1> README DO SIMULADOR DE INCENDIO </h1>

Aluno: Kairo Henrique Ferreira Martins 
Professor: Michel Pires da Silva  
Disciplina: Algoritmos e Estruturas de Dados I  

---

## Introdução

Este trabalho de aquecimento implementa um simulador de propagação de incêndio em uma floresta modelada por matriz, aliado à movimentação de um animal que busca refúgio em áreas seguras ou fontes de água. A motivação é exercitar conceitos de matrizes dinâmicas, estruturas de dados e controle de fluxo em C++.

---

## Funcionamento geral do programa

- **Configuração:** um arquivo `Config.hpp` (na pasta `include/`) define parâmetros como vento, número máximo de iterações e exibição em console.  
- **Leitura:** a função em `LerArquivo.hpp/cpp` carrega uma matriz de `input.dat`.  
- **Loop principal:** em `main.cpp`, repete `executarIteracao()` até que o fogo se apague, o animal morra ou atinja o limite de iterações.  
- **Ordem de ações:** primeiro o animal se move, depois o fogo se propaga; se o fogo alcançar o animal, uma “segunda chance” de movimento é aplicada imediatamente.  
- **Saída:** cada iteração gera logs e a matriz atualizada em `output.dat`; em tela, a posição do animal é marcada por `A`.

---

## Sobre os Codigos

Em cada ciclo:

1. `executarIteracao(iter)`  
2. `moverAnimal()`  
3. `propagarFogo()`  
4. se o animal ficou em célula em chamas, chamar `moverAnimal()` novamente

---

## Arquivo de Configuração (`Config.hpp`)

```cpp
#pragma once
namespace Config {
    // Direções de vento: [Cima, Baixo, Esquerda, Direita]
    const bool Vento[4]        = { true, true, true, true };
    // Limite de iterações para evitar loops infinitos
    const int  MaxIteracoes    = 1000;
    // Controla exibição da simulação no console
    const bool exibirNoConsole = true;
}
```

- Usei um array `bool[4]` para ativar/desativar cada direção de vento.  
- `MaxIteracoes` me dá segurança contra casos patológicos.  
- `exibirNoConsole` facilita debug sem alterar código-fonte.

---

## Leitura de Dados (`LerArquivo.hpp` / `LerArquivo.cpp`)

```cpp
// LerArquivo.hpp
#pragma once
#include <vector>
#include <fstream>
std::vector<std::vector<int>> lerArquivo(std::ifstream& arquivo, int linhas, int colunas);

// LerArquivo.cpp
#include "LerArquivo.hpp"
std::vector<std::vector<int>> lerArquivo(std::ifstream& arquivo, int linhas, int colunas) {
    std::vector<std::vector<int>> m(linhas, std::vector<int>(colunas));
    for (int i = 0; i < linhas; ++i)
        for (int j = 0; j < colunas; ++j)
            arquivo >> m[i][j];
    return m;
}
```

**`vector<vector<int>>`?**  
- Garante flexibilidade para matrizes N×M de qualquer tamanho.  
- A alocação dinâmica evita estouro de pilha.

---

## Classe `Floresta` (`Floresta.hpp` / `Floresta.cpp`)

### Variáveis principais

- `matriz` e `matrizProxima` (`vector<vector<int>>`): estado atual e futuro da floresta.  
- `struct Animal { x, y, passos, aguaEncontrada, iteracaoPreso, tempoEmSeguro }`: encapsula todos os dados do agente.  
- `logs` (`vector<string>`): armazena descrições de cada mudança para saída e debug.

### Construtor

```cpp
Floresta::Floresta(int n, int m, int fr, int fc,
                   const std::vector<std::vector<int>>& initialMatrix)
  : linhas(n), colunas(m),
    matriz(initialMatrix), matrizProxima(initialMatrix),
    linhaFogo(fr), colunaFogo(fc),
    iteracaoAtual(0), animalVivo(true)
{
    matriz[fr][fc] = 2;  // inicia fogo
    // busca primeira célula 0 para posicionar o animal
    bool ok = false;
    for (int i = 0; i < n && !ok; ++i)
      for (int j = 0; j < m && !ok; ++j)
        if (matriz[i][j] == 0) {
          animal.x = i; animal.y = j; ok = true;
        }
    if (!ok) animalVivo = false;
    animal.passos = animal.aguaEncontrada = 0;
    animal.iteracaoPreso = -1; animal.tempoEmSeguro = 0;
}
```
 
- Duas matrizes evitam que novas chamas afetem propagações na mesma iteração.  
- Posicionar fogo e animal no construtor garante estado inicial consistente.

---

### `moverAnimal()`

```cpp
void Floresta::moverAnimal() {
    if (!animalVivo) return;
    int x = animal.x, y = animal.y;
    // se estiver em area vazia, espera até 3 iterações
    if (matriz[x][y] == 0 && animal.tempoEmSeguro++ < 3) return;
    else if (matriz[x][y] == 0) animal.tempoEmSeguro = 0;

    struct D { int dx, dy; };
    std::vector<D> dirs = {{-1,0},{1,0},{0,-1},{0,1}};
    std::vector<std::pair<int,int>> opcs;

    for (size_t i = 0; i < dirs.size(); ++i) {
      int nx = x + dirs[i].dx, ny = y + dirs[i].dy;
      if (nx<0||nx>=linhas||ny<0||ny>=colunas) continue;
      int v = matriz[nx][ny], p;
      if      (v == 4) p = 0;      // água
      else if (v == 0 || v == 1) p = 1;  // seguro
      else if (v == 3) p = 2;      // queimado
      else continue;               // fogo não é opção
      opcs.emplace_back(p, i);
    }
    if (opcs.empty()) {
      animalVivo = false;
      animal.iteracaoPreso = iteracaoAtual;
      return;
    }
    std::sort(opcs.begin(), opcs.end());
    int idx = opcs[0].second;
    animal.x += dirs[idx].dx;
    animal.y += dirs[idx].dy;
    animal.passos++;

    // se encontrou água, converte célula e arredores
    if (matriz[animal.x][animal.y] == 4) {
      animal.aguaEncontrada++;
      matriz[animal.x][animal.y] = 0;
      logs.push_back("(" + std::to_string(animal.x) + "," +
                     std::to_string(animal.y) + ") vira 0");
      for (auto& d : dirs) {
        int ax = animal.x + d.dx, ay = animal.y + d.dy;
        if (ax>=0&&ax<linhas&&ay>=0&&ay<colunas&&matriz[ax][ay]!=1) {
          matriz[ax][ay] = 1;
          logs.push_back("(" + std::to_string(ax) + "," +
                         std::to_string(ay) + ") vira 1");
        }
      }
    }
}
```
- Prioridade explícita de escolhas garante comportamento previsível.  
- “Tempo em seguro” evita que o animal se mova inutilmente em células vazias eternamente.

---

### `propagarFogo()`

```cpp
void Floresta::propagarFogo() {
    matrizProxima = matriz;
    for (int i = 0; i < linhas; ++i)
      for (int j = 0; j < colunas; ++j)
        if (matriz[i][j] == 2) {
          matrizProxima[i][j] = 3;
          std::vector<std::pair<int,int>> dirs;
          if (Config::Vento[0]) dirs.emplace_back(-1,0);
          if (Config::Vento[1]) dirs.emplace_back( 1,0);
          if (Config::Vento[2]) dirs.emplace_back( 0,-1);
          if (Config::Vento[3]) dirs.emplace_back( 0, 1);
          for (auto& d : dirs) {
            int x = i + d.first, y = j + d.second;
            if (x>=0&&x<linhas&&y>=0&&y<colunas&&matriz[x][y]==1) {
              matrizProxima[x][y] = 2;
              // grava log de propagação
              logs.push_back("(" + std::to_string(x) + "," +
                             std::to_string(y) + ") vira 2");
            }
          }
        }
    matriz = matrizProxima;
}
```
 
- Vetor de direções condicionado em `Config::Vento` torna a propagação parametrizável.  
- Matriz auxiliar mantém integridade dos estados.

---

### `executarIteracao(int iter)`

```cpp
void Floresta::executarIteracao(int iter) {
    iteracaoAtual = iter;
    logs.clear();
    logs.push_back("Animal em: (" + std::to_string(animal.x) +
                   "," + std::to_string(animal.y) + ")");
    moverAnimal();
    propagarFogo();
    if (matriz[animal.x][animal.y] == 2) moverAnimal();  // segunda chance
}
```

---

## Loop Principal (`main.cpp`)

```cpp
int main() {
    std::ifstream fin("data/input.dat");
    std::ofstream fout("data/output.dat");
    int n,m,fr,fc;
    fin >> n >> m >> fr >> fc;
    auto mat = lerArquivo(fin, n, m);
    Floresta f(n,m,fr,fc,mat);
    int it = 1;
    while (!f.terminou() && it <= Config::MaxIteracoes) {
      f.executarIteracao(it);
      if (Config::exibirNoConsole) {
        // imprime logs e matriz com 'A'
      }
      for (auto& log : f.getLogs()) fout << log << "\n";
      // escreve matriz e 'A' em fout
      ++it;
    }
    // estatísticas finais
    fout << "Passos: "        << f.getPassosAnimal()   << "\n"
         << "Água encontrada: "<< f.getAguaEncontrada() << "\n"
         << "Sobreviveu: "     << (f.estaAnimalVivo() ? "Sim":"Não") << "\n";
    if (!f.estaAnimalVivo())
      fout << "Iteração preso: " << f.getIteracaoPreso() << "\n";
}
```

---

## Estrutura do projeto

```
include/
  Config.hpp
  Floresta.hpp
  LerArquivo.hpp

src/
  Floresta.cpp
  LerArquivo.cpp
  main.cpp

data/
  input.dat
  output.dat

INPUTS TESTES/ (Varios testes realizados)
  input.dat
  output.dat

Makefile
```

---

## Movimentação do Animal

1. **Água (4):** prioridade máxima — reseta célula para 0 e umidifica vizinhas.  
2. **Seguro (0 ou 1):** movimento intercalado; espera até 3 iterações em 0 para estabilidade.  
3. **Queimado (3):** última opção, caso não haja mais alternativas.  
4. **Impossibilitado:** `animalVivo = false`, registra `iteracaoPreso`.

---

## Segunda Chance

Se após `propagarFogo()` o animal estiver em célula com valor `2`, chama `moverAnimal()` outra vez para que ele possa escapar, mas apenas se houver vizinhos válidos.

---

## Representação Visual do Incêndio

- **Sem vento ou com vento em todas direções:** fogo em todas as ortogonais.  
- **Vento seletivo:** respeita o vetor `Config::Vento`, propagando apenas nas direções permitidas.

---

## Casos de Teste





## Ideias de Melhoria de Performance


Em vez de varrer toda a matriz, manter um `queue<pair<int,int>>` com células em chamas

Usar OpenMP para processar vizinhanças de fogo simultaneamente.  


---

## Compilação

Requisitos: Linux, `g++` (C++17), `make`

```bash
make clean
make
make run
```

---

## Autores

Kairo Henrique Ferreira Martins
```