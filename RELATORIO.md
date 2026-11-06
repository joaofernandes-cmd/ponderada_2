# Relatório · Atividade ponderada de Computação 2 - Semáforo com dois modos de operação

**Nome:** João Glauco Fernandes Araújo de Freitas (joao.fernandes@sou.inteli.edu.br) · **Turma:** T26
**Grupo de bancada:** Grupo 2 · **Colegas de bancada:** n/d
**Previsão registrada em:** 05/11/2026, 18:21:49
**Primeira leitura importada em:** 05/11/2026, 18:46:15 · previsão registrada antes da primeira leitura

## 1. Contextualizar

### Semáforo básico: tabela de previsão

| Item | Previsto |
|---|---|
| Último dígito do RA | 7 |
| Tempo de vermelho | 12 s |
| Tempo de verde | 6 s |
| Tempo de amarelo | 2 s |
| Ciclo completo | 20 s |
| Tensão de alimentação considerada | 3.3 V |
| Resistor de cada LED | 330 Ω |
| Corrente esperada no LED vermelho | 3.94 mA |
| Corrente esperada no LED amarelo | 3.64 mA |
| Corrente esperada no LED verde | 3.33 mA |
| Desvio aceitável na temporização | 5 % |

- **Por que esse desvio é aceitável:** Porque o erro do ESP32 é praticamente zero, o cristal tem precisão de cerca de 0,005%, e o código usa millis() sem delay(), então cada fase atrasa no máximo alguns milissegundos, o desvio real deve ficar abaixo de 0,1%, ou seja, cerca de 12 ms em 12 s.

### Modo noturno

| Item | Valor |
|---|---|
| Transições indevidas toleráveis por minuto | 0 |
| Limiar previsto (0 a 4095) | 100 |

- **Por que esse número de transições:** Pensando em um cenário real, caso houvesse alguma troca de modo de operação sem motivo geraria um risco a vida de qualquer motorista no trânsito, a exemplo de que um motorista pode ver o amarelo piscando no lugar do vermelho e atravessar o cruzamento.
- **Origem do limiar previsto:** Veio das medições de bancada com o script de calibração, com a luz ambiente a leitura ficou em  ~330, com o LDR coberto foi 0 e com lanterna ~3500. O limiar central de 150 fica no meio entre escuro e ambiente, e a histerese de 50 evita oscilação perto do limite.
- **O que delego à IA:** Pedi a IA auxilio na adaptação do meu código que eu montei no Arduino para o ESP32 e a integração com o script de calibração, a troca de delay() por millis(), a sugerindo a aplicação da histerese e confirmação de leituras, e a ajuda para interpretar os logs do Monitor Serial.

## 2. Consultar

- **Ferramenta:** Claude

### Semáforo básico

- **Pergunta sobre o circuito ou o código:** Pensando na utilização do LDR, seguindo a sua documentação oficial, como funciona ele funciona internamente, e por qual motivo ele não apresenta polaridade ou seja um catodo e um anodo?
- **Resposta da IA:** O LDR é um resistor de sulfeto de cádmio (CdS) cuja resistência cai com a luz, por fotocondutividade. No escuro há poucos portadores de carga livres e a resistência é alta (MΩ). A luz libera elétrons no semicondutor, e a resistência cai para centenas de ohms. Ele não tem polaridade porque não possui junção PN, ao contrário de LEDs e diodos. É um único material, com dois eletrodos iguais, e conduz igualmente nos dois sentidos. No circuito, o LDR fica entre 3,3 V e o GPIO34, formando um divisor de tensão com o resistor fixo. Por isso a leitura do ADC sobe com luz (~3.500–4.000) e cai a 0 no escuro.

### Modo noturno

- **Pergunta:** Como evitar que um sistema fique alternando de estado quando a leitura do sensor está perto de um limiar? Me explique conceitualmente.
- **Estratégia sugerida:** Em vez de um único limiar, usam-se dois limiares diferentes: um para entrar no estado e outro para sair. Entre os dois fica uma “faixa morta”, em que o sistema mantém o estado atual. Assim, pequenas variações perto do limite não provocam troca. Para reforçar, exige-se que a leitura fique do outro lado do limiar por várias leituras seguidas (por exemplo, 5 leituras em 1 s) antes de trocar. Isso ignora picos e ruídos isolados do sensor.
- **Nome da técnica:** Histerese
- **Limiar sugerido pela IA:** 2000

## 3. Confrontar

### Semáforo básico: corrente contra o datasheet

- **LED usado:** LED difuso 5 mm genérico
- **Corrente máxima no datasheet:** 25 mA
- **Resistor sugerido pela IA:** n/d
- **Resistor adotado:** 330 Ω
- **Por quê:** Adotei pois era o único resistor compatível presente no meu kit, algo que havia validado com o professor Roberto em aulas anteriores e que também confirmei com a IA.

Verificações:
- OK: LED vermelho: 3,94 mA previstos, dentro do limite de 25,0 mA do datasheet.
- OK: LED amarelo: 3,64 mA previstos, dentro do limite de 25,0 mA do datasheet.
- OK: LED verde: 3,33 mA previstos, dentro do limite de 25,0 mA do datasheet.

### Modo noturno: tabela de calibração

Leitura bruta do conversor de 12 bits (0 a 4095).

| Condição | n | Mínimo | Máximo | Média | Desvio |
|---|---|---|---|---|---|
| ambiente | 30 | 194 | 199 | 196,6 | 1,5 |
| sombra | 30 | 0 | 0 | 0,0 | 0,0 |
| lanterna | 30 | 2.994 | 3.794 | 3.515,2 | 195,1 |

- **Sentido:** a leitura sobe com a luz
- **Maior ruído numa mesma condição:** 5 contagens

Verificações:
- OK: ambiente iluminado: 30 leituras, de 194 a 199.
- OK: sombreado pela mão: 30 leituras, de 0 a 0.
- Atenção: sombreado pela mão: todas as leituras são múltiplas de 10. Leitura bruta do conversor raramente é redonda; tabela com valores redondos sem medição recebe zero.
- OK: lanterna: 30 leituras, de 2.994 a 3.794.
- OK: Sentido detectado: a leitura sobe com a luz (lanterna acima da sombra).
- OK: Região entre as faixas: leituras de 0 a 194. Maior ruído numa mesma condição: 5 contagens.
- OK: Limiar previsto (100) fica entre a faixa de sombra e a de ambiente.
- Atenção: Limiar sugerido pela IA (2.000) fica além da faixa de ambiente, do lado da lanterna, e o modo noturno entraria com a sala iluminada.

**Divergência explicada:** Na primeira versão do código, a IA colocou um limiar provisório de 2000, com a orientação de ajustar depois da calibração. Nas minhas medições, o valor certo ficou em torno de 100, uma diferença de cerca de 1900 pontos, e o sentido estava invertido: no meu circuito, a leitura desce no escuro. Isso aconteceu porque a leitura depende da montagem, do resistor, da peça e da luz da sala, por isso, o limiar precisou sair das minhas medições.

Gráfico: `grafico-calibracao.png`.

<details><summary>Leituras brutas (90)</summary>

```csv
condicao,leitura
ambiente,197
ambiente,195
ambiente,197
ambiente,197
ambiente,197
ambiente,199
ambiente,194
ambiente,198
ambiente,195
ambiente,195
ambiente,196
ambiente,198
ambiente,197
ambiente,198
ambiente,199
ambiente,195
ambiente,197
ambiente,194
ambiente,198
ambiente,195
ambiente,194
ambiente,198
ambiente,194
ambiente,197
ambiente,198
ambiente,198
ambiente,198
ambiente,196
ambiente,197
ambiente,197
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
sombra,0
lanterna,2994
lanterna,3142
lanterna,3184
lanterna,3153
lanterna,3261
lanterna,3514
lanterna,3678
lanterna,3697
lanterna,3732
lanterna,3794
lanterna,3742
lanterna,3664
lanterna,3639
lanterna,3600
lanterna,3655
lanterna,3607
lanterna,3617
lanterna,3637
lanterna,3633
lanterna,3600
lanterna,3569
lanterna,3556
lanterna,3544
lanterna,3472
lanterna,3408
lanterna,3490
lanterna,3503
lanterna,3485
lanterna,3471
lanterna,3414
```

</details>

## 4. Construir

### Semáforo básico

- **Pinos (vermelho, amarelo, verde):** GPIO 25, 26, 27
- **Código:** https://github.com/joaofernandes-cmd/ponderada_2.git
- **Ciclo programado:** vermelho 12 s, verde 6 s, amarelo 2 s, repetindo

### Modo noturno (amarelo intermitente, com histerese)

| Item | Valor |
|---|---|
| Limiar de entrada no modo noturno | 60 |
| Limiar de saída do modo noturno | 140 |
| Folga entre os limiares | 80 contagens |

Verificações:
- OK: Ordem coerente com o sentido: entrada do lado escuro, saída do lado claro.
- OK: Limiar de entrada (60) fica entre a faixa de sombra e a de ambiente.
- OK: Limiar de saída (140) fica entre a faixa de sombra e a de ambiente.
- OK: Folga de 80 contagens, maior que o ruído medido de 5.

**Como os limiares saíram das medições:** Quando fiz a calibração, percebi que com o LDR coberto a leitura ficava sempre em 0, enquanto com a luz da sala ela variava pouco, entre 194 e 199, com média de 196. Com isso, escolhi 60 como limiar de entrada e 140 como limiar de saída, um de cada lado do meio entre essas duas faixas, deixando uma folga de 80 pontos entre eles, bem maior que a pequena variação que medi, para que o semáforo não ficasse trocando de modo à toa e só mudasse quando eu realmente cobrisse ou descobrisse o sensor.

## 5. Comprovar

### Semáforo básico: tabela de previsão com a medição

| Fase | Previsto (s) | Medido com cronômetro (s) | Desvio |
|---|---|---|---|
| vermelho | 12 | 12,02 | +0,2 % |
| verde | 6 | 5,86 | -2,3 % |
| amarelo | 2 | 1,95 | -2,5 % |
| ciclo | 20 | 20,02 | +0,1 % |

Critério de aceitação: 5 %.

Verificações:
- OK: vermelho: desvio de +0,2 %, dentro do critério de 5,0 %.
- OK: verde: desvio de -2,3 %, dentro do critério de 5,0 %.
- OK: amarelo: desvio de -2,5 %, dentro do critério de 5,0 %.
- OK: ciclo: desvio de +0,1 %, dentro do critério de 5,0 %.

- **Explicação das divergências:** Nenhuma das fases passaram do critério.
- **Vídeo do ciclo completo (30 a 60 s):** https://youtu.be/lXPfTcxiTxI

#### Cartão de falha

- **Sintoma:** n/d
- **Causa mais provável:** n/d
- **Teste para confirmar:** n/d

### Modo noturno

- **Vídeo da transição:** https://youtu.be/VtkAtLZOLwY
- **Trocas de modo observadas:** 2 (critério declarado: 0 por minuto)
- **Por que a histerese impede a oscilação:** Pois ela define limiares de entrada e saída do modo noturno, para que o semáforo troque de modo apenas quando os valores registrados por ele passarem daquelas faixas.

ADR em `ADR.md`.
