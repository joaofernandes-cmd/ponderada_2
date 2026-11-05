// Atividade ponderada de Computação 2 - Semáforo com dois modos de operação.
// Modo dia:   vermelho 12 s -> verde 6 s -> amarelo 2 s (ciclo normal)
// Modo noite: vermelho e verde apagados, amarelo piscando
//
// Comandos no Monitor Serial (115200, digite a letra e Enter):
//   a = medir ambiente   s = medir sombra   l = medir lanterna
//   t = testar LEDs (um por vez)
//   d = forçar DIA       n = forçar NOITE   x = automático (LDR)
//   v = liga/desliga status a cada 1 s  (DESATIVADO - ver trechos comentados)

// ---------- Pinos ----------
const int PINO_LDR     = 34;   // ADC1: continua lendo com o Wi-Fi ligado
const int LED_VERMELHO = 25;
const int LED_AMARELO  = 26;
const int LED_VERDE    = 27;

// ---------- Calibração (script do professor) ----------
const unsigned long INTERVALO_MS = 200;   // uma leitura a cada 200 ms
const int LEITURAS_POR_CONDICAO = 30;     // 30 leituras = 6 s por condição
const char *condicao = "ambiente";
int contagem = LEITURAS_POR_CONDICAO;     // começa parado, esperando a primeira letra
long somaCondicao = 0;

// ---------- Limiar dia/noite (valores medidos na bancada) ----------
// Ambiente ~330, LDR coberto = 0, lanterna ~3500 -> a leitura DESCE no escuro
const int LIMIAR    = 150;   // meio do caminho entre escuro (0) e ambiente (~330)
const int HISTERESE = 50;    // entra na noite abaixo de 100, volta ao dia acima de 200
const int CONFIRMACOES = 5;  // 5 leituras seguidas (1 s) para trocar de modo

// ---------- Tempos do semáforo ----------
const unsigned long TEMPO_VERMELHO = 12000;
const unsigned long TEMPO_VERDE    = 6000;
const unsigned long TEMPO_AMARELO  = 2000;
const unsigned long TEMPO_PISCA    = 500;   // pisca do amarelo no modo noite

// ---------- Estado ----------
enum Fase { FASE_VERMELHO, FASE_VERDE, FASE_AMARELO };
enum Operacao { AUTOMATICO, FORCA_DIA, FORCA_NOITE };
Fase fase = FASE_VERMELHO;
Operacao operacao = AUTOMATICO;
bool modoNoite = false;
bool amareloAceso = false;
// bool statusLigado = true;          // status desativado
int ultimaLeituraLDR = 0;
int contadorTroca = 0;
unsigned long inicioFase = 0;
unsigned long ultimoPisca = 0;
unsigned long ultimaLeitura = 0;
// unsigned long ultimoStatus = 0;    // status desativado

void acenderLeds(bool vm, bool am, bool vd) {
  digitalWrite(LED_VERMELHO, vm);
  digitalWrite(LED_AMARELO,  am);
  digitalWrite(LED_VERDE,    vd);
}

void testarLeds() {
  const int pinos[] = {LED_VERMELHO, LED_AMARELO, LED_VERDE};
  const char *nomes[] = {"VERMELHO (GPIO25)", "AMARELO (GPIO26)", "VERDE (GPIO27)"};
  Serial.println("# --- teste dos LEDs ---");
  acenderLeds(LOW, LOW, LOW);
  for (int i = 0; i < 3; i++) {
    Serial.print("# deve estar aceso: ");
    Serial.println(nomes[i]);
    digitalWrite(pinos[i], HIGH);
    delay(1000);
    digitalWrite(pinos[i], LOW);
  }
}

// A leitura DESCE no escuro. Histerese: entrar na noite exige < 100; sair exige > 200.
bool estaEscuro(int leitura) {
  if (modoNoite) return leitura < LIMIAR + HISTERESE;  // continua noite enquanto < 200
  else           return leitura < LIMIAR - HISTERESE;  // vira noite quando < 100
}

void iniciarModoDia(unsigned long agora) {
  fase = FASE_VERMELHO;
  inicioFase = agora;
  acenderLeds(HIGH, LOW, LOW);
  Serial.println("# modo DIA");
}

void iniciarModoNoite(unsigned long agora) {
  amareloAceso = true;
  ultimoPisca = agora;
  acenderLeds(LOW, HIGH, LOW);
  Serial.println("# modo NOITE");
}

void atualizarModoDia(unsigned long agora) {
  unsigned long decorrido = agora - inicioFase;
  switch (fase) {
    case FASE_VERMELHO:
      if (decorrido >= TEMPO_VERMELHO) { fase = FASE_VERDE; inicioFase = agora; acenderLeds(LOW, LOW, HIGH); }
      break;
    case FASE_VERDE:
      if (decorrido >= TEMPO_VERDE) { fase = FASE_AMARELO; inicioFase = agora; acenderLeds(LOW, HIGH, LOW); }
      break;
    case FASE_AMARELO:
      if (decorrido >= TEMPO_AMARELO) { fase = FASE_VERMELHO; inicioFase = agora; acenderLeds(HIGH, LOW, LOW); }
      break;
  }
}

void atualizarModoNoite(unsigned long agora) {
  if (agora - ultimoPisca >= TEMPO_PISCA) {
    amareloAceso = !amareloAceso;
    digitalWrite(LED_AMARELO, amareloAceso);
    ultimoPisca = agora;
  }
}

// ---------- Status a cada 1 s (DESATIVADO) ----------
// void mostrarStatus() {
//   const char *nomesFase[] = {"VERMELHO", "VERDE", "AMARELO"};
//   const char *nomesOp[] = {"auto", "forcado DIA", "forcado NOITE"};
//   Serial.print("# LDR=");    Serial.print(ultimaLeituraLDR);
//   Serial.print(modoNoite ? " (volta ao dia se > " : " (vira noite se < ");
//   Serial.print(modoNoite ? LIMIAR + HISTERESE : LIMIAR - HISTERESE);
//   Serial.print(") modo=");   Serial.print(modoNoite ? "NOITE" : "DIA");
//   Serial.print(" [");        Serial.print(nomesOp[operacao]); Serial.print("]");
//   if (!modoNoite) { Serial.print(" fase="); Serial.print(nomesFase[fase]); }
//   Serial.println();
// }

void setup() {
  Serial.begin(115200);
  delay(500);
  analogReadResolution(12);              // escala de 0 a 4095

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERDE,    OUTPUT);

  testarLeds();

  Serial.println("# comandos: a s l (calibrar) | t (teste LEDs) | d n x (dia/noite/auto)");
  Serial.println("condicao,leitura");

  iniciarModoDia(millis());
}

void loop() {
  unsigned long agora = millis();

  // Comandos
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'a') { condicao = "ambiente"; contagem = 0; somaCondicao = 0; }
    if (c == 's') { condicao = "sombra";   contagem = 0; somaCondicao = 0; }
    if (c == 'l') { condicao = "lanterna"; contagem = 0; somaCondicao = 0; }
    if (c == 't') { testarLeds(); agora = millis(); modoNoite ? iniciarModoNoite(agora) : iniciarModoDia(agora); }
    if (c == 'd') { operacao = FORCA_DIA;   Serial.println("# forcando DIA"); }
    if (c == 'n') { operacao = FORCA_NOITE; Serial.println("# forcando NOITE"); }
    if (c == 'x') { operacao = AUTOMATICO;  Serial.println("# modo automatico (LDR)"); }
    // if (c == 'v') { statusLigado = !statusLigado; }   // status desativado
  }

  // Leitura do LDR a cada 200 ms (sem travar o semáforo)
  if (agora - ultimaLeitura >= INTERVALO_MS) {
    ultimaLeitura = agora;
    int leitura = analogRead(PINO_LDR);
    ultimaLeituraLDR = leitura;

    if (contagem < LEITURAS_POR_CONDICAO) {
      Serial.print(condicao);
      Serial.print(",");
      Serial.println(leitura);
      somaCondicao += leitura;
      contagem++;
      if (contagem == LEITURAS_POR_CONDICAO) {
        Serial.print("# fim da condicao - media: ");
        Serial.println(somaCondicao / LEITURAS_POR_CONDICAO);
      }
    }

    bool deveSerNoite;
    if (operacao == FORCA_DIA)        deveSerNoite = false;
    else if (operacao == FORCA_NOITE) deveSerNoite = true;
    else                              deveSerNoite = estaEscuro(leitura);

    // Só troca de modo após CONFIRMACOES leituras seguidas pedindo a troca
    if (deveSerNoite != modoNoite) {
      contadorTroca++;
      if (contadorTroca >= CONFIRMACOES || operacao != AUTOMATICO) {
        contadorTroca = 0;
        modoNoite = deveSerNoite;
        if (modoNoite) iniciarModoNoite(agora);
        else           iniciarModoDia(agora);
      }
    } else {
      contadorTroca = 0;
    }
  }

  // ---------- Status a cada 1 s (DESATIVADO) ----------
  // if (statusLigado && contagem >= LEITURAS_POR_CONDICAO && agora - ultimoStatus >= 1000) {
  //   ultimoStatus = agora;
  //   mostrarStatus();
  // }

  // Semáforo
  if (modoNoite) atualizarModoNoite(agora);
  else           atualizarModoDia(agora);
}