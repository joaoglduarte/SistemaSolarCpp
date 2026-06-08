#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <memory>

const double PI = 3.14159265358979323846;

using namespace std;

class CorpoCeleste {
private:
    string nome;
    double raioReal;
    double distanciaRealdoSol; // (Agora pode ser lido como Distância do Astro Central)
    double velocidadeAngular; 
    double anguloAtual; 
    double velocidadeRotacao; 
    float  rotacaoAtual;       
    
    sf::Texture textura;
    sf::Sprite sprite;

    // NOVO: Um vetor para guardar os satélites (luas) deste corpo
    vector<unique_ptr<CorpoCeleste>> satelites;

    static double escalaTamanho;
    static double escalaDistancia;
    static double velocidadeSimulacao;

public:
    CorpoCeleste(string n, double raio, double dist, double velOrbita, string caminhoImagem, double velRotacao = 0.0)
        : nome(n), raioReal(raio), distanciaRealdoSol(dist), velocidadeAngular(velOrbita), anguloAtual(0.0), 
          velocidadeRotacao(velRotacao), rotacaoAtual(0.0f) {
        
        if (!textura.loadFromFile(caminhoImagem)) {
            cerr << "Erro ao carregar a imagem: " << caminhoImagem << endl;
        }
        
        sprite.setTexture(textura);
        sprite.setOrigin(textura.getSize().x / 2.0f, textura.getSize().y / 2.0f);
    }
    
    virtual ~CorpoCeleste() = default;

    // NOVO: Método para acoplar luas a este planeta
    void adicionarSatelite(unique_ptr<CorpoCeleste> satelite) {
        satelites.push_back(move(satelite));
    }

    virtual void atualizar(double deltaTime) {
        // --- TRANSLAÇÃO ---
        anguloAtual += velocidadeAngular * deltaTime * velocidadeSimulacao;
        anguloAtual = fmod(anguloAtual, 2 * PI);
        if (anguloAtual < 0) anguloAtual += 2 * PI; // Garante que o ângulo orbital seja sempre positivo

        // --- ROTAÇÃO ---
        rotacaoAtual += velocidadeRotacao * deltaTime * velocidadeSimulacao;
        rotacaoAtual = fmod(rotacaoAtual, 360.0);
        if (rotacaoAtual < 0) rotacaoAtual += 360.0; // Garante que o giro do sprite seja válido no SFML

        // NOVO: Atualiza todas as luas deste planeta
        for (auto& sat : satelites) {
            sat->atualizar(deltaTime);
        }
    }

    virtual void renderizar(sf::RenderWindow& janela, double centroX, double centroY) {
        // A posição X e Y calculada Deste Astro
        double posX = centroX + (distanciaRealdoSol * escalaDistancia) * cos(anguloAtual);
        double posY = centroY + (distanciaRealdoSol * escalaDistancia) * sin(anguloAtual);
        sprite.setPosition(posX, posY);

        double diametroDesejado = (raioReal * escalaTamanho) * 2.0;
        float fatorEscalaX = static_cast<float>(diametroDesejado / textura.getSize().x);
        float fatorEscalaY = static_cast<float>(diametroDesejado / textura.getSize().y);
        sprite.setScale(fatorEscalaX, fatorEscalaY);

        sprite.setRotation(rotacaoAtual);
        janela.draw(sprite);

        // NOVO: Renderiza as luas, passando a posição ATUAL deste planeta como o novo centro
        for (auto& sat : satelites) {
            sat->renderizar(janela, posX, posY);
        }
    }

    // Métodos para identificar o corpo e pegar sua posição na tela
    string getNome() const { return nome; }
    sf::Vector2f getPosicaoMundo() const { return sprite.getPosition(); }

    // Busca recursiva: verifica se é este planeta, ou se é uma das luas dele
    CorpoCeleste* buscarSatelite(const string& nomeAlvo) {
        if (nome == nomeAlvo) return this; // É este astro!
        
        for (auto& sat : satelites) {
            CorpoCeleste* encontrado = sat->buscarSatelite(nomeAlvo);
            if (encontrado) return encontrado; // Achou em uma das luas!
        }
        return nullptr; // Não achou nada nesta ramificação
    }

    static void setEscalaTamanho(double escala) { escalaTamanho = escala; }
    static void setEscalaDistancia(double escala) { escalaDistancia = escala; }
    static void setVelocidadeSimulacao(double vel) { velocidadeSimulacao = vel; }
    static double getVelocidadeSimulacao() { return velocidadeSimulacao; }
    
};

double CorpoCeleste::escalaTamanho = 1.0;
double CorpoCeleste::escalaDistancia = 1.0;
double CorpoCeleste::velocidadeSimulacao = 1.0;

class SistemaSolar {
private:
    vector<unique_ptr<CorpoCeleste>> corpos;

public:
    void adicionarCorpo(unique_ptr<CorpoCeleste> corpo) {
        corpos.push_back(move(corpo));
    }
    void atualizarSistema(double deltaTime) {
        for (auto& corpo : corpos) {
            corpo->atualizar(deltaTime);
        }
    }
    void renderizarSistema(sf::RenderWindow& intranet, double centroX, double centroY) {
        for (auto& corpo : corpos) {
            corpo->renderizar(intranet, centroX, centroY);
        }
    }
    CorpoCeleste* buscarCorpo(const string& nomeAlvo) {
        for (auto& corpo : corpos) {
            CorpoCeleste* encontrado = corpo->buscarSatelite(nomeAlvo);
            if (encontrado) return encontrado;
        }
        return nullptr;
    }
};


int main() {
    sf::RenderWindow janela(sf::VideoMode(900, 900), "Simulador Sistema Solar - POO C++");
    sf::Clock relogio;
    SistemaSolar sistema;

    sf::View camera(sf::FloatRect(0, 0, 900, 900));
    camera.setCenter(450.0f, 450.0f);
    
    bool arrastando = false;
    sf::Vector2i posicaoMouseAnterior;

    // --- ESCALAS DA SIMULAÇÃO ---
    // Guardamos os padrões em constantes
    const double ESCALA_TAM_PADRAO = 0.002;
    const double ESCALA_DIST_PADRAO = 0.0000015;
    const double FATOR_TEMPO_PADRAO = 1000000.0;

    // Variáveis que vão mudar durante a execução
    double escalaTam = ESCALA_TAM_PADRAO;          
    double escalaDist = ESCALA_DIST_PADRAO;     
    double fatorTempo = FATOR_TEMPO_PADRAO;     

    CorpoCeleste::setEscalaTamanho(escalaTam); 
    CorpoCeleste::setEscalaDistancia(escalaDist); 
    CorpoCeleste::setVelocidadeSimulacao(fatorTempo);

    // 1. sol
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Sol", 40000.0, 0.0, 0.0, "assets/sol.png", 0.0));

    // 2. planetas rochosos
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Mercurio", 2439.0, 57900000.0, 0.0000008, "assets/mercurio.png", 10.0));
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Venus", 6051.0, 108200000.0, 0.0000003, "assets/venus.png", -8.0)); // Rotação retrógrada

    auto terra = make_unique<CorpoCeleste>("Terra", 6371.0, 149600000.0, 0.0000002, "assets/terra.png", 100.0);
    terra->adicionarSatelite(make_unique<CorpoCeleste>("Lua", 2000.0, 25000000.0, 0.000002, "assets/lua.png", 50.0));
    sistema.adicionarCorpo(move(terra));

    auto marte = make_unique<CorpoCeleste>("Marte", 3389.0, 227900000.0, 0.0000001, "assets/marte.png", 90.0);
    marte->adicionarSatelite(make_unique<CorpoCeleste>("Fobos", 1000.0, 12000000.0, 0.000003, "assets/fobos.png", 60.0));
    marte->adicionarSatelite(make_unique<CorpoCeleste>("Deimos", 800.0, 18000000.0, 0.0000025, "assets/deimos.png", 55.0));
    sistema.adicionarCorpo(move(marte));

    // 3. cinturao de asteroides
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Vesta", 262.0, 353000000.0, 0.00000008, "assets/vesta.png", 200.0));
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Ceres", 473.0, 413000000.0, 0.00000007, "assets/ceres.png", 150.0));

    //4. gigantes gasosos
    auto jupiter = make_unique<CorpoCeleste>("Jupiter", 69911.0, 778500000.0, 0.000000016, "assets/jupiter.png", 250.0);
    // 4.5 Luas Galileanas
    jupiter->adicionarSatelite(make_unique<CorpoCeleste>("Io", 1821.0, 120000000.0, 0.000005, "assets/io.png", 40.0));
    jupiter->adicionarSatelite(make_unique<CorpoCeleste>("Europa", 1560.0, 160000000.0, 0.000004, "assets/europa.png", 35.0));
    jupiter->adicionarSatelite(make_unique<CorpoCeleste>("Ganimedes", 2634.0, 210000000.0, 0.000003, "assets/ganimedes.png", 30.0));
    jupiter->adicionarSatelite(make_unique<CorpoCeleste>("Calisto", 2410.0, 270000000.0, 0.000002, "assets/callisto.png", 25.0));
    sistema.adicionarCorpo(move(jupiter));

    auto saturno = make_unique<CorpoCeleste>("Saturno", 58232.0, 1429000000.0, 0.000000006, "assets/saturno.png", 230.0);
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Mimas", 198.0, 110000000.0, 0.000004, "assets/mimas.png", 50.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Enceladus", 252.0, 140000000.0, 0.0000035, "assets/enceladus.png", 45.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Tethys", 531.0, 170000000.0, 0.000003, "assets/tethys.png", 40.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Dione", 561.0, 200000000.0, 0.0000025, "assets/dione.png", 35.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Rhea", 763.0, 240000000.0, 0.000002, "assets/rhea.png", 30.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Titan", 2574.0, 300000000.0, 0.0000015, "assets/titan.png", 20.0));
    saturno->adicionarSatelite(make_unique<CorpoCeleste>("Iapetus", 734.0, 360000000.0, 0.000001, "assets/iapetus.png", 15.0));
    sistema.adicionarCorpo(move(saturno));

    auto urano = make_unique<CorpoCeleste>("Urano", 25362.0, 2871000000.0, 0.000000002, "assets/urano.png", -150.0);
    urano->adicionarSatelite(make_unique<CorpoCeleste>("Miranda", 235.0, 60000000.0, 0.000003, "assets/miranda.png", 40.0));
    urano->adicionarSatelite(make_unique<CorpoCeleste>("Ariel", 578.0, 80000000.0, 0.0000025, "assets/ariel.png", 35.0));
    urano->adicionarSatelite(make_unique<CorpoCeleste>("Umbriel", 584.0, 100000000.0, 0.000002, "assets/umbriel.png", 30.0));
    urano->adicionarSatelite(make_unique<CorpoCeleste>("Titania", 788.0, 130000000.0, 0.0000015, "assets/titania.png", 25.0));
    urano->adicionarSatelite(make_unique<CorpoCeleste>("Oberon", 761.0, 160000000.0, 0.000001, "assets/oberon.png", 20.0));
    sistema.adicionarCorpo(move(urano));

    auto netuno = make_unique<CorpoCeleste>("Netuno", 24622.0, 4498000000.0, 0.000000001, "assets/netuno.png", 140.0);
    netuno->adicionarSatelite(make_unique<CorpoCeleste>("Triton", 1353.0, 80000000.0, -0.000002, "assets/triton.png", 30.0)); // Órbita retrógrada
    sistema.adicionarCorpo(move(netuno));

    // 5. cometas e transnetunianos (TNOs)
    
    // Halley (Órbita simplificada)
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Halley", 5.0, 2600000000.0, 0.000000003, "assets/halley.png", 50.0));

    auto plutao = make_unique<CorpoCeleste>("Plutao", 1188.0, 5906000000.0, 0.0000000006, "assets/plutao.png", -20.0);
    plutao->adicionarSatelite(make_unique<CorpoCeleste>("Charon", 606.0, 15000000.0, 0.000005, "assets/charon.png", 10.0));
    sistema.adicionarCorpo(move(plutao));

    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Haumea", 816.0, 6452000000.0, 0.0000000005, "assets/haumea.png", 500.0));
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Makemake", 715.0, 6850000000.0, 0.0000000004, "assets/makemake.png", 80.0));
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Quaoar", 555.0, 6500000000.0, 0.00000000045, "assets/quaoar.png", 60.0));
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Eris", 1163.0, 10120000000.0, 0.0000000003, "assets/eris.png", 40.0));
    
    //sedn
    sistema.adicionarCorpo(make_unique<CorpoCeleste>("Sedna", 497.0, 78000000000.0, 0.00000000005, "assets/sedna.png", 30.0));

    //FOCO DA CÂMERA ---
    // Lista dos astros que poderemos focar apertando TAB
    vector<string> alvosCamera = {"Sol", "Mercurio", "Venus", "Terra", "Lua", "Marte", "Fobos", "Deimos", "Ceres", "Vesta", "Jupiter", "Saturno", "Urano", "Netuno", "Plutao", "Haumea", "Makemake", "Quaoar", "Eris", "Sedna", "Halley", "Charon", "Io", "Europa", "Ganimedes", "Calisto", "Mimas", "Enceladus", "Tethys", "Dione", "Rhea", "Titan", "Iapetus", "Miranda", "Ariel", "Umbriel", "Titania", "Oberon", "Triton"};
    int indiceAlvo = 0;
    CorpoCeleste* astroFocado = nullptr; // nullptr significa câmera livre

    while (janela.isOpen()) {
        sf::Event evento;
        while (janela.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                janela.close();
            }
            
            // (ZOOM E ARRASTA) ---
            else if (evento.type == sf::Event::MouseWheelScrolled) {
                if (evento.mouseWheelScroll.delta > 0) camera.zoom(0.9f);
                else if (evento.mouseWheelScroll.delta < 0) camera.zoom(1.1f);
            }
            else if (evento.type == sf::Event::MouseButtonPressed) {
                if (evento.mouseButton.button == sf::Mouse::Left) {
                    arrastando = true;
                    posicaoMouseAnterior = sf::Mouse::getPosition(janela);
                }else if (evento.type == sf::Event::MouseButtonPressed) {
                if (evento.mouseButton.button == sf::Mouse::Left) {
                    arrastando = true;
                    posicaoMouseAnterior = sf::Mouse::getPosition(janela);
                    
                    // Se o usuário clicar para arrastar, solta o foco da câmera
                    if (astroFocado != nullptr) {
                        astroFocado = nullptr;
                        cout << "Câmera livre" << endl;
                    }
                }
            }
            }
            else if (evento.type == sf::Event::MouseButtonReleased) {
                if (evento.mouseButton.button == sf::Mouse::Left) arrastando = false;
            }
            else if (evento.type == sf::Event::MouseMoved) {
                if (arrastando) {
                    sf::Vector2i posicaoMouseAtual = sf::Mouse::getPosition(janela);
                    sf::Vector2f posMundoAnterior = janela.mapPixelToCoords(posicaoMouseAnterior, camera);
                    sf::Vector2f posMundoAtual = janela.mapPixelToCoords(posicaoMouseAtual, camera);
                    camera.move(posMundoAnterior - posMundoAtual);
                    posicaoMouseAnterior = posicaoMouseAtual;
                }
            }
            
            // --- CONTROLO DO TECLADO UNIFICADO ---
            else if (evento.type == sf::Event::KeyPressed) {
                
                // 1. Controle do Tempo (Setas)
                if (evento.key.code == sf::Keyboard::Up) {
                    fatorTempo *= 2.0; 
                    CorpoCeleste::setVelocidadeSimulacao(fatorTempo);
                    cout << "Tempo: " << fatorTempo << "x" << endl;
                }
                else if (evento.key.code == sf::Keyboard::Down) {
                    fatorTempo /= 2.0; 
                    CorpoCeleste::setVelocidadeSimulacao(fatorTempo);
                    cout << "Tempo: " << fatorTempo << "x" << endl;
                }

                // 2. Controle de Escala de TAMANHO (Teclas Q e A)
                else if (evento.key.code == sf::Keyboard::Q) {
                    escalaTam *= 1.1; 
                    CorpoCeleste::setEscalaTamanho(escalaTam);
                }
                else if (evento.key.code == sf::Keyboard::A) {
                    escalaTam /= 1.1; 
                    CorpoCeleste::setEscalaTamanho(escalaTam);
                }

                // 3. Controle de Escala de DISTÂNCIA (Teclas W e S)
                else if (evento.key.code == sf::Keyboard::W) {
                    escalaDist *= 1.1; 
                    CorpoCeleste::setEscalaDistancia(escalaDist);
                }
                else if (evento.key.code == sf::Keyboard::S) {
                    escalaDist /= 1.1; 
                    CorpoCeleste::setEscalaDistancia(escalaDist);
                }
                // 4. Controle de Foco da Câmera (Tecla TAB)
                else if (evento.key.code == sf::Keyboard::Tab) {
                    astroFocado = sistema.buscarCorpo(alvosCamera[indiceAlvo]);
                    if (astroFocado) {
                        cout << "Câmera focada em: " << astroFocado->getNome() << endl;
                    }
                    // Vai para o próximo índice (se passar do tamanho do vetor, volta a zero)
                    indiceAlvo = (indiceAlvo + 1) % alvosCamera.size();
                }
                // 5. Botão de Reset Total (Tecla R)
                else if (evento.key.code == sf::Keyboard::R) {
                    // 1. Restaura as variáveis locais aos padrões originais
                    escalaTam = ESCALA_TAM_PADRAO;
                    escalaDist = ESCALA_DIST_PADRAO;
                    fatorTempo = FATOR_TEMPO_PADRAO;

                    // 2. Envia os valores restaurados para a física do sistema
                    CorpoCeleste::setEscalaTamanho(escalaTam);
                    CorpoCeleste::setEscalaDistancia(escalaDist);
                    CorpoCeleste::setVelocidadeSimulacao(fatorTempo);

                    // 3. Restaura a Câmera
                    astroFocado = nullptr; // Solta o foco do planeta
                    camera.setSize(900.0f, 900.0f); // Tira todo o zoom
                    camera.setCenter(450.0f, 450.0f); // Centraliza no Sol (ponto de origem)

                    cout << "Simulação restaurada para os padrões originais!" << endl;
                }
            }
        }
        double deltaTime = relogio.restart().asSeconds();
        sistema.atualizarSistema(deltaTime);

        // --- APLICA O FOCO DA CÂMERA ---
        if (astroFocado != nullptr) {
            // A câmera é movida magicamente para o centro exato do astro a cada frame
            camera.setCenter(astroFocado->getPosicaoMundo());
        }

        janela.clear(sf::Color(10, 10, 15));
        janela.setView(camera);
        sistema.renderizarSistema(janela, 450.0, 450.0);
        janela.display();
    }

    return 0;
}