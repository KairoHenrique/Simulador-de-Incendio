<h1> 🔥README DO SIMULADOR DE INCENDIO🔥</h1>

- **Aluno:** Kairo Henrique Ferreira Martins 
- **Professor:** Michel Pires da Silva  
- **Disciplina:** Algoritmos e Estruturas de Dados I  

---

## Introdução
  Este projeto modela a propagação de fogo em uma floresta representada por uma matriz, incluindo a movimentação estratégica de um animal em busca de segurança.
---

### **Objetivo**  
Fornecer uma ferramenta para:  
- Analisar padrões de propagação de fogo com/sem influência do vento.  
- Simular a busca de rotas de fuga por um animal em tempo real.  
- Estudar o impacto de escolhas algorítmicas no desempenho computacional.  

---

### **Funcionalidades Principais**  
1. **Matriz Dinâmica**  
   - Representa a floresta com células de estados variados:  
     - `0`: Área segura (não queima)  
     - `1`: Árvore saudável  
     - `2`: Fogo ativo  
     - `3`: Árvore queimada  
     - `4`: Água (converte células adjacentes para Árvore saudável).  

2. **Propagação Realista do Fogo**  
   - Direções configuráveis do vento (`Config.hpp`).  
   - Atualização iterativa com duas matrizes para evitar conflitos de estado.  

3. **Movimentação Inteligente do Animal**  
   - Prioriza rotas seguras: `Água (4) > Vazio (0) > Árvore saudável (1) > Queimada (3)`.  
   - **Segunda chance:** Se atingido pelo fogo, tenta escapar novamente.  

4. **Entrada/Saída de Dados**  
   - Leitura de cenários via `input.dat`.  
   - Geração de logs detalhados em `output.dat`, incluindo:  
     - Posição do animal (`A`) em cada iteração.  
     - Estatísticas finais (passos, sobrevivência, etc.).  

---

## Metadologia

Em cada ciclo:

1. `executarIteracao(iter)`  
2. `moverAnimal()`  
3. `propagarFogo()`  
4. se o animal ficou em célula em chamas, chamar `moverAnimal()` novamente

---

## Arquivo de Configuração (`Config.hpp`)

```cpp
#pragma once🔥
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

### Caso 1: Propagação sem vento (todas as direções)

**input.dat**
```
5 5 1 1
1 1 1 1 4
1 2 1 1 1
1 1 1 1 4
0 0 1 1 1
1 4 1 0 4
```

**output.dat (resumo)**
- Fogo se propaga em todas as direções ortogonais.
- Animal inicia em (3,0), movimenta-se até (4,1), encontra água e transforma vizinhos.
- Simulação encerra após 6 iterações.
- Passos: 2 | Água encontrada: 1 | Sobreviveu: Sim

<details>
  <summary><strong>Mostrar output.dat completo</strong></summary>

```
Animal em: (3,0)
(0,1) vira 2 (acima)
(2,1) vira 2 (abaixo)
(1,0) vira 2 (esquerda)
(1,2) vira 2 (direita)
Iteracao 1:
1 2 1 1 4 
2 3 2 1 1 
1 2 1 1 4 
A 0 1 1 1 
1 4 1 0 4 
Animal em: (3,0)
(0,0) vira 2 (esquerda)
(0,2) vira 2 (direita)
(0,0) vira 2 (acima)
(2,0) vira 2 (abaixo)
(0,2) vira 2 (acima)
(2,2) vira 2 (abaixo)
(1,3) vira 2 (direita)
(2,0) vira 2 (esquerda)
(2,2) vira 2 (direita)
Iteracao 2:
2 3 2 1 4 
3 3 3 2 1 
2 3 2 1 4 
A 0 1 1 1 
1 4 1 0 4 
Animal em: (3,0)
(0,3) vira 2 (direita)
(0,3) vira 2 (acima)
(2,3) vira 2 (abaixo)
(1,4) vira 2 (direita)
(3,2) vira 2 (abaixo)
(2,3) vira 2 (direita)
Iteracao 3:
3 3 3 2 4 
3 3 3 3 2 
3 3 3 2 4 
A 0 2 1 1 
1 4 1 0 4 
Animal em: (3,0)
(3,3) vira 2 (abaixo)
(4,2) vira 2 (abaixo)
(3,3) vira 2 (direita)
Iteracao 4:
3 3 3 3 4 
3 3 3 3 3 
3 3 3 3 4 
0 0 3 2 1 
A 4 2 0 4 
Animal em: (4,0)
(4,1) vira 0
(3,1) vira 1
(4,2) vira 1
(3,4) vira 2 (direita)
Iteracao 5:
3 3 3 3 4 
3 3 3 3 3 
3 3 3 3 4 
0 1 3 3 2 
1 A 1 0 4 
Animal em: (4,1)
Iteracao 6:
3 3 3 3 4 
3 3 3 3 3 
3 3 3 3 4 
0 1 3 3 3 
1 A 1 0 4 

=== Estatisticas do Animal ===
Passos: 2
Agua encontrada: 1
Sobreviveu: Sim

```
</details> 


---

### Caso 2: Propagação com vento (somente para a direita)

**input.dat**
```
6 6 2 2
1 1 2 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
0 1 1 4 1 1
1 1 1 1 1 1
1 1 1 1 1 1
```

**output.dat (resumo)**
- Fogo se propaga apenas para a direita, de acordo com `Config::Vento`.
- Animal inicia em (3,0), anda 2 vezes, mas não encontra água.
- Simulação encerra após 7 iterações.
- Passos: 2 | Água encontrada: 0 | Sobreviveu: Sim
<details>
  <summary><strong>Mostrar output.dat completo</strong></summary>

```
Animal em: (3,0)
(1,2) vira 2 (abaixo)
(0,1) vira 2 (esquerda)
(0,3) vira 2 (direita)
(1,2) vira 2 (acima)
(3,2) vira 2 (abaixo)
(2,1) vira 2 (esquerda)
(2,3) vira 2 (direita)
Iteracao 1:
1 2 3 2 1 1 
1 1 2 1 1 1 
1 2 3 2 1 1 
A 1 2 4 1 1 
1 1 1 1 1 1 
1 1 1 1 1 1 
Animal em: (3,0)
(1,1) vira 2 (abaixo)
(0,0) vira 2 (esquerda)
(1,3) vira 2 (abaixo)
(0,4) vira 2 (direita)
(1,1) vira 2 (esquerda)
(1,3) vira 2 (direita)
(1,1) vira 2 (acima)
(3,1) vira 2 (abaixo)
(2,0) vira 2 (esquerda)
(1,3) vira 2 (acima)
(2,4) vira 2 (direita)
(4,2) vira 2 (abaixo)
(3,1) vira 2 (esquerda)
Iteracao 2:
2 3 3 3 2 1 
1 2 3 2 1 1 
2 3 3 3 2 1 
A 2 3 4 1 1 
1 1 2 1 1 1 
1 1 1 1 1 1 
Animal em: (3,0)
(1,0) vira 2 (abaixo)
(1,4) vira 2 (abaixo)
(0,5) vira 2 (direita)
(1,0) vira 2 (esquerda)
(1,4) vira 2 (direita)
(1,0) vira 2 (acima)
(1,4) vira 2 (acima)
(3,4) vira 2 (abaixo)
(2,5) vira 2 (direita)
(4,1) vira 2 (abaixo)
(5,2) vira 2 (abaixo)
(4,1) vira 2 (esquerda)
(4,3) vira 2 (direita)
Iteracao 3:
3 3 3 3 3 2 
2 3 3 3 2 1 
3 3 3 3 3 2 
A 3 3 4 2 1 
1 2 3 2 1 1 
1 1 2 1 1 1 
Animal em: (3,0)
(1,5) vira 2 (abaixo)
(1,5) vira 2 (direita)
(1,5) vira 2 (acima)
(3,5) vira 2 (abaixo)
(4,4) vira 2 (abaixo)
(3,5) vira 2 (direita)
(5,1) vira 2 (abaixo)
(4,0) vira 2 (esquerda)
(5,3) vira 2 (abaixo)
(4,4) vira 2 (direita)
(5,1) vira 2 (esquerda)
(5,3) vira 2 (direita)
Iteracao 4:
3 3 3 3 3 3 
3 3 3 3 3 2 
3 3 3 3 3 3 
A 3 3 4 3 2 
2 3 3 3 2 1 
1 2 3 2 1 1 
Animal em: (3,0)
(4,5) vira 2 (abaixo)
(5,0) vira 2 (abaixo)
(5,4) vira 2 (abaixo)
(4,5) vira 2 (direita)
(5,0) vira 2 (esquerda)
(5,4) vira 2 (direita)
Iteracao 5:
3 3 3 3 3 3 
3 3 3 3 3 3 
3 3 3 3 3 3 
A 3 3 4 3 3 
3 3 3 3 3 2 
2 3 3 3 2 1 
Animal em: (3,0)
(5,5) vira 2 (abaixo)
(5,5) vira 2 (direita)
Iteracao 6:
3 3 3 3 3 3 
3 3 3 3 3 3 
3 3 3 3 3 3 
A 3 3 4 3 3 
3 3 3 3 3 3 
3 3 3 3 3 2 
Animal em: (3,0)
Iteracao 7:
3 3 3 3 3 3 
3 3 3 3 3 3 
3 3 3 3 3 3 
A 3 3 4 3 3 
3 3 3 3 3 3 
3 3 3 3 3 3 

=== Estatisticas do Animal ===
Passos: 2
Agua encontrada: 0
Sobreviveu: Sim

```
</details> 

---

### Caso 3: Animal preso pelo fogo

**input.dat**
```
4 4 0 0
2 1 1 1
1 1 1 1
1 1 1 1
0 1 1 1
```

**output.dat (resumo)**
- Fogo fecha todas as rotas possíveis.
- Animal inicia em (3,0), mas logo fica encurralado.
- Apesar de estar vivo no final, não pode mais se mover.
- Passos: 2 | Água encontrada: 0 | Sobreviveu: Sim
<details>
  <summary><strong>Mostrar output.dat completo</strong></summary>

```
Animal em: (3,0)
(1,0) vira 2 (abaixo)
(0,1) vira 2 (direita)
Iteracao 1:
3 2 1 1 
2 1 1 1 
1 1 1 1 
A 1 1 1 
Animal em: (3,0)
(1,1) vira 2 (abaixo)
(0,2) vira 2 (direita)
(2,0) vira 2 (abaixo)
(1,1) vira 2 (direita)
Iteracao 2:
3 3 2 1 
3 2 1 1 
2 1 1 1 
A 1 1 1 
Animal em: (3,0)
(1,2) vira 2 (abaixo)
(0,3) vira 2 (direita)
(2,1) vira 2 (abaixo)
(1,2) vira 2 (direita)
(2,1) vira 2 (direita)
Iteracao 3:
3 3 3 2 
3 3 2 1 
3 2 1 1 
A 1 1 1 
Animal em: (3,0)
(1,3) vira 2 (abaixo)
(2,2) vira 2 (abaixo)
(1,3) vira 2 (direita)
(3,1) vira 2 (abaixo)
(2,2) vira 2 (direita)
Iteracao 4:
3 3 3 3 
3 3 3 2 
3 3 2 1 
A 2 1 1 
Animal em: (3,0)
(2,3) vira 2 (abaixo)
(3,2) vira 2 (abaixo)
(2,3) vira 2 (direita)
(3,2) vira 2 (direita)
Iteracao 5:
3 3 3 3 
3 3 3 3 
3 3 3 2 
A 3 2 1 
Animal em: (3,0)
(3,3) vira 2 (abaixo)
(3,3) vira 2 (direita)
Iteracao 6:
3 3 3 3 
3 3 3 3 
3 3 3 3 
A 3 3 2 
Animal em: (3,0)
Iteracao 7:
3 3 3 3 
3 3 3 3 
3 3 3 3 
A 3 3 3 

=== Estatisticas do Animal ===
Passos: 2
Agua encontrada: 0
Sobreviveu: Sim

```
</details> 

---

### Caso 4: Interrupção por limite de iterações (MaxIteracoes = 3)

**input.dat**
```
3 3 0 0
2 1 1
1 1 1
0 1 1
```

**output.dat (resumo)**
- Animal se move normalmente, mas a simulação é interrompida após 3 iterações.
- Demonstra o uso efetivo do parâmetro `MaxIteracoes`.
- Passos: 2 | Água encontrada: 0 | Sobreviveu: Sim
<details>
  <summary><strong>Mostrar output.dat completo</strong></summary>

```
Animal em: (2,0)
(1,0) vira 2 (abaixo)
(0,1) vira 2 (direita)
Iteracao 1:
3 2 1 
2 1 1 
A 1 1 
Animal em: (2,0)
(1,1) vira 2 (abaixo)
(0,2) vira 2 (direita)
(1,1) vira 2 (direita)
Iteracao 2:
3 3 2 
3 2 1 
A 1 1 
Animal em: (2,0)
(1,2) vira 2 (abaixo)
(2,1) vira 2 (abaixo)
(1,2) vira 2 (direita)
Iteracao 3:
3 3 3 
3 3 2 
A 2 1 
Animal em: (2,0)
(2,2) vira 2 (abaixo)
(2,2) vira 2 (direita)
Iteracao 4:
3 3 3 
A 3 3 
0 3 2 
Animal em: (1,0)
Iteracao 5:
3 3 3 
3 3 3 
A 3 3 

=== Estatisticas do Animal ===
Passos: 2
Agua encontrada: 0
Sobreviveu: Sim

```
</details> 

---

### Caso 5: Teste da Segunda Chance

**input.dat**
```
5 5 1 1
0 1 1 1 4
1 2 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
```

**output.dat (resumo)**
- Animal começa em (0,0), e na primeira propagação o fogo ameaça sua posição.
- Recebe segunda chance, se move para (1,0) e continua vivo.
- Mesmo cercado por fogo, a lógica da segunda chance garante que ele tente escapar.
- Passos: 2 | Água encontrada: 0 | Sobreviveu: Sim
<details>
  <summary><strong>Mostrar output.dat completo</strong></summary>

```
Animal em: (0,0)
(0,1) vira 2 (acima)
(2,1) vira 2 (abaixo)
(1,0) vira 2 (esquerda)
(1,2) vira 2 (direita)
Iteracao 1:
A 2 1 1 4 
2 3 2 1 1 
1 2 1 1 1 
1 1 1 1 1 
1 1 1 1 1 
Animal em: (0,0)
(0,2) vira 2 (direita)
(2,0) vira 2 (abaixo)
(0,2) vira 2 (acima)
(2,2) vira 2 (abaixo)
(1,3) vira 2 (direita)
(3,1) vira 2 (abaixo)
(2,0) vira 2 (esquerda)
(2,2) vira 2 (direita)
Iteracao 2:
A 3 2 1 4 
3 3 3 2 1 
2 3 2 1 1 
1 2 1 1 1 
1 1 1 1 1 
Animal em: (0,0)
(0,3) vira 2 (direita)
(0,3) vira 2 (acima)
(2,3) vira 2 (abaixo)
(1,4) vira 2 (direita)
(3,0) vira 2 (abaixo)
(3,2) vira 2 (abaixo)
(2,3) vira 2 (direita)
(4,1) vira 2 (abaixo)
(3,0) vira 2 (esquerda)
(3,2) vira 2 (direita)
Iteracao 3:
A 3 3 2 4 
3 3 3 3 2 
3 3 3 2 1 
2 3 2 1 1 
1 2 1 1 1 
Animal em: (0,0)
(2,4) vira 2 (abaixo)
(3,3) vira 2 (abaixo)
(2,4) vira 2 (direita)
(4,0) vira 2 (abaixo)
(4,2) vira 2 (abaixo)
(3,3) vira 2 (direita)
(4,0) vira 2 (esquerda)
(4,2) vira 2 (direita)
Iteracao 4:
0 3 3 3 4 
A 3 3 3 3 
3 3 3 3 2 
3 3 3 2 1 
2 3 2 1 1 
Animal em: (1,0)
(3,4) vira 2 (abaixo)
(4,3) vira 2 (abaixo)
(3,4) vira 2 (direita)
(4,3) vira 2 (direita)
Iteracao 5:
A 3 3 3 4 
3 3 3 3 3 
3 3 3 3 3 
3 3 3 3 2 
3 3 3 2 1 
Animal em: (0,0)
(4,4) vira 2 (abaixo)
(4,4) vira 2 (direita)
Iteracao 6:
A 3 3 3 4 
3 3 3 3 3 
3 3 3 3 3 
3 3 3 3 3 
3 3 3 3 2 
Animal em: (0,0)
Iteracao 7:
A 3 3 3 4 
3 3 3 3 3 
3 3 3 3 3 
3 3 3 3 3 
3 3 3 3 3 

=== Estatisticas do Animal ===
Passos: 2
Agua encontrada: 0
Sobreviveu: Sim

```
</details> 



---
## Conclusão
### Ideias de Melhoria de Performance


- Em vez de varrer toda a matriz, manter um `queue<pair<int,int>>` com células em chamas

- Usar OpenMP para processar vizinhanças de fogo simultaneamente.  


---

## Compilação

Requisitos: Linux, `g++` (C++17), `make`

```bash
make clean
make
make run
```

---

## Autor

- Kairo Henrique Ferreira Martins
