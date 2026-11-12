# ADR · Atividade ponderada de Computação 2 - Semáforo com dois modos de operação

**Data:** 12/11/2026
**Autor:** João Glauco Fernandes Araújo de Freitas (joao.fernandes@sou.inteli.edu.br)
**Turma:** T26

## Contexto

Semáforo de três LEDs com o plano do RA (dígito 7) e modo noturno automático, detectado por LDR. Calibração: média de 197 no ambiente, 0 na sombra e 3.515 com lanterna. Limiares adotados: entrada 60 e saída 140.

## Decisão

Decidi usar como base o script que eu já tinha montado no Arduino, mantendo a mesma lógica de dois modos e o pisca do amarelo com millis(), e adaptar para o ESP32 com os pinos 25, 26 e 27 e a leitura no GPIO34. Também mantive a histerese no lugar do limiar único que eu usava antes.

## Alternativa considerada

Considerei apagar todo o trecho do status, o que deixaria o código mais curto, mas abri mão disso porque o status foi o que me mostrou o valor do LDR em tempo real e me ajudou a perceber problemas no limiar, então perder essa ferramenta dificultaria ajustes futuros.

## Sugestão de IA rejeitada ou corrigida

Corrigi os limiares de histerese sugeridos pelo Claude, que fazia o modo noturno entrar abaixo de 100 e sair só acima de 200, e passei a usar entrada em 60 e saída em 140.

## Razão técnica

Nas minhas medições, a luz ambiente caiu para cerca de 196, abaixo do limiar de saída de 200, e o semáforo entrava no modo noturno e não voltava mais ao modo diurno quando eu descobria o LDR, só saindo com a lanterna. Com os novos limiares, a folga de 80 pontos continua bem maior que o ruído de 5 pontos que medi, e a leitura ambiente fica acima do limiar de saída.
