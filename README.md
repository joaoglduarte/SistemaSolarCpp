# Simulador do Sistema Solar 2D (C++ & SFML)

Um simulador físico e interativo do Sistema Solar desenvolvido inteiramente em **C++** utilizando a biblioteca gráfica **SFML**. Este projeto foi construído como um estudo prático de **Programação Orientada a Objetos (POO)**, mecânica orbital e manipulação de câmeras 2D.

## 🚀 Funcionalidades Técnicas
* **Motor Orbital:** Cálculo em tempo real de órbitas usando trigonometria e física de translação e rotação.
* **Padrão Composite:** Arquitetura de software que permite o acoplamento hierárquico de astros (ex: Luas orbitam planetas, que por sua vez orbitam o Sol) de forma recursiva.
* **Paradoxo da Escala:** Controles dinâmicos e independentes para a escala geométrica (tamanho dos corpos) e escala astronômica (distância orbital), contornando o problema visual do espaço profundo.
* **Câmera Interativa:** Sistema de Viewport com suporte a arrasto (pan), zoom e *Tracking* (trava de foco automático em corpos em movimento).

## 🎮 Controles da Simulação

A simulação é totalmente interativa. Você pode explorar o espaço livremente ou focar em eventos astronômicos específicos.

### Mouse (Navegação)
* **Scroll (Roda do Mouse):** Zoom In / Zoom Out.
* **Clique Esquerdo + Arrasto:** Move a câmera livremente pelo espaço. *(Atenção: Arrastar a tela desativa automaticamente o modo de Foco).*

### Teclado (Física e Tempo)
* **`Seta para Cima`**: Acelera a passagem do tempo (Time Warp +).
* **`Seta para Baixo`**: Desacelera a passagem do tempo (Time Warp -).
* **`Q` e `A`**: Aumenta / Diminui o raio (tamanho) visual dos astros.
* **`W` e `S`**: Expande / Contrai a distância das órbitas.
* **`TAB`**: Cicla o foco da câmera, travando a visão automaticamente em planetas específicos (Sol, Terra, Marte, Júpiter, etc.).
* **`R`**: Reset de emergência. Restaura a câmera, o tempo e as escalas geométricas para os valores originais.

## 🛠️ Como Compilar e Executar

Este projeto foi desenvolvido em ambiente Linux e requer o compilador `g++` e os módulos de desenvolvimento do SFML instalados.

### Pré-requisitos (Debian/Ubuntu)
Instale a biblioteca gráfica pelo terminal:
```bash
sudo apt update
sudo apt install libsfml-dev
```

Na raiz do projeto, execute o comando de linkagem:
```bash
g++ corpos.cpp -o simulador -lsfml-graphics -lsfml-window -lsfml-system
```

Garanta que a pasta assets/ (contendo todas as sprites dos astros em .png) está no mesmo diretório do executável criado, e inicie o programa:
```bash
./simulador
```
