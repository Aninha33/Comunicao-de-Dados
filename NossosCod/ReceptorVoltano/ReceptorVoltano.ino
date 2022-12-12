#define LDR_PIN A0
#define LDR_THRESHOLD 500
#define CLOCK_PERIOD_MS 500

int ultimo_sinal;

void deco_NRZL(int8_t *recebidinho_msg, uint8_t *bin_msg){
  // Decodificador NRZ-L (led apagado (-1) == 0, led ligado (1) == 1)
 for (uint8_t i = 0; i < 11; i++) bin_msg[i] = recebidinho_msg[i] == -1 ? 1 : 0;
}//deco_NRZL

void deco_NRZI(int8_t *recebidinho_msg, uint8_t *bin_msg){
  // Decodificador NRZ-I 
  for (uint8_t i = 0; i < 11; i++) {
    // Se for o primeiro bit trasmitido, faz como no NRZ-I mas salva 
    // Ultimo valor transmitido
    if (i != 0) { 
        // Se o novo valor é igual o valor atual, 0, senao (inversao) é 1
      if (recebidinho_msg[i] == ultimo_sinal) bin_msg[i] = 0;
      
      else {     
        bin_msg[i] = 1;
        ultimo_sinal = recebidinho_msg[i];
      }//else
    } else {     
      bin_msg[i] = recebidinho_msg[i] == -1 ? 1 : 0;
      ultimo_sinal = recebidinho_msg[i];
    }//else
  }//for
}//deco_NRZI

bool check_checksum(uint8_t *bin_msg, uint8_t *bin_char) {
  // Verifica o CRC
  // Código CRC utilizado
  uint8_t code[] = {1, 1, 0, 1};
  uint8_t tamanhozinho_code = sizeof(code) / sizeof(code[0]);

  // Passa a mensagem recebida para um vetor auxiliar realizar a validação
  uint8_t aux[11];
  for (int i = 0; i < 11; i++) aux[i] = bin_msg[i];
  
  uint8_t tamanhozinho_aux = sizeof(aux) / sizeof(aux[0]);

  // Faz a operação bit por bit
  // Começa no bit 0 e vai até o bit 7
  // Realiza operação XOR (bits iguais 0, bits diferentes 1) entre a mensagem recebida
  // Código CRC
  for (uint8_t i = 0; i <= tamanhozinho_aux - tamanhozinho_code;){
    for (uint8_t j = 0; j < tamanhozinho_code; j++) aux[i + j] = aux[i + j] == code[j] ? 0 : 1;
  
    for (i; i < tamanhozinho_aux && aux[i] != 1; i++);
  
  }//for

  // Verifica se todos os bits do resultado da operação são 0, senão, retorna falso
  // Extrai os bits do caractere (bit 0 até 7)
  for (uint8_t i = 0; i < tamanhozinho_aux; i++) {
    if (aux[i] == 1) return false; 
    if (i < 8) bin_char[i] = bin_msg[i];
  }//for

  return true;
}//check_checksum

uint8_t asciizinho(uint8_t *bin_char) {
  // Binário para cedimal para pegar o código ascii do caractere
  int ascii = 0;
  for (uint8_t i = 0; i <= 7; i++) {
    double double_pow = pow((float)2, (float)(7 - i));
    ascii += bin_char[i] * ceil(double_pow);
  }//for

  return (uint8_t)ascii;
}//asciizinho

bool tem_maiszinho(long int *ino) {
  // Verifica se mais um fragmento de mensagem será enviado
  // Se o emissor deixar o led aceso por 2 ciclos, mais um fragmento será enviado
  int8_t recebidinho = 0;
  int8_t agorinha = 0;
  for (uint8_t i = 0; i < 2; i++) {
    *ino = millis();
    recebidinho = lux_ldr();
    if (agorinha == recebidinho && recebidinho == 1 && agorinha != 0 ) return true; 
    
    agorinha = recebidinho;
    delay(CLOCK_PERIOD_MS);
  }//for

  return false;
}//tem_maiszinho

void pedacito_msg_funcao(int8_t *pedacito_msg){
  // Pega um fragmento da mensagem (11 caracteres consecutivos)
int i=0;
do{
pedacito_msg[i] = lux_ldr();
    Serial.print(pedacito_msg[i]);
    Serial.print("\t");
    delay(CLOCK_PERIOD_MS);
    i++;
}while(i<11);

/*
  for (uint8_t i = 0; i < 11; i++) {
    pedacito_msg[i] = lux_ldr();
    Serial.print(pedacito_msg[i]);
    Serial.print("\t");
    delay(CLOCK_PERIOD_MS);
  }//for
  */
}//pedacito_msg_funcao

int8_t lux_ldr() {
  // Pega o código do LDR, se for maior que o threshold (led acesso) retorna 1, senão -1
  uint16_t ldr_valorzinho = analogRead(LDR_PIN);
  if (ldr_valorzinho <= LDR_THRESHOLD) return -1; 
  else return 1;  
}//lux_ldr

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
}//setup

void loop() {
  int8_t recebidinho = 0;    // Variavel auxiliar do sinal recebido
  int8_t agorinha = 0;      // Variavel auxiliar do ultimo sinal recebido
  long int ino = 0;        // Variavel auxiliar para calcular o tempo inicial decorrido
  long int voltano = 0;   // Variavel auxiliar para calcular o tempo final decorrido
  
  do {
    // Para manter a sincronização, verifica o tempo da operação para ajustar o delay
    ino = millis();
    // Pega o valor lido pelo LDR
    recebidinho = lux_ldr();

    // Emissor avisa uma transmissao jogando dois sinais altos consecutivos
    if (agorinha == recebidinho && recebidinho == 1 && agorinha != 0) {
      Serial.println("Transmissão sendo recebida...");
      voltano = millis();
      // Delay para manter a sincronização
      delay(CLOCK_PERIOD_MS - (voltano - ino));

      // Vetor com a mensagem a ser recebida
      // Limitação: Tamanho máximo da mensagem é fixo
      int tamarindo_msgzinha = 100;
      int8_t *mensagenzinha[tamarindo_msgzinha];

      // Pega o primeiro fragmento da mensagem
      int8_t *pedacito_msg = (int8_t *)malloc(sizeof(int8_t) * 11);
      pedacito_msg_funcao(pedacito_msg);
      mensagenzinha[0] = pedacito_msg;
      Serial.println();

      // Pega os demais fragmentos da mensagem
      int prox_pedacito = 1;
      while (tem_maiszinho(&ino)){
        voltano = millis();
        // Delay para manter a sincronização
        delay(CLOCK_PERIOD_MS - (voltano - ino));
        
        // Pega o novo fragmento da mensagem
        int8_t *pedacito_msg = (int8_t *)malloc(sizeof(int8_t) * 11);
        pedacito_msg_funcao(pedacito_msg);
        mensagenzinha[prox_pedacito] = pedacito_msg;
        prox_pedacito++;
        Serial.println();
      }//while

      Serial.println();
      Serial.println("Mensagem recebida:");
      
      // Processa os dados recebidos (cada fragmento)
      for (int i = 0; i < prox_pedacito; i++){
        // Decodifica
        uint8_t *bin_msg = (uint8_t *)malloc(sizeof(uint8_t) * 11);
        
      //auxiliar para escolher o NRZI OU NRZL  
      int aux_NR = 0;
          if(aux_NR ==0){
          deco_NRZI(mensagenzinha[i], bin_msg);
          }else{//if
          deco_NRZL(mensagenzinha[i], bin_msg);
          }//else   
        

        // Verifica se o CRC está correto
        uint8_t *bin_char = (uint8_t *)malloc(sizeof(uint8_t) * 8);
        if (check_checksum(bin_msg, bin_char)) {
          // Se sim printa o caractere na tela
          uint8_t ascii = asciizinho(bin_char);
          Serial.print((char)ascii);
        }//if
      
        // Senao mostra um erro (caractere não chegou direito) 
        else Serial.print("[fragmento corrompido]");   

        // Libera as variáveis dinânmicas
        free(bin_msg);
        free(bin_char);
      }//for
      Serial.println();

      // Libera os elementos dinâmicos do vetor da mensagem
      for (int i = 0; i <= prox_pedacito; i++) free(mensagenzinha[i]);

      free(mensagenzinha);
    }//if

    // Atualiza ultimo sinal recebido
    agorinha = recebidinho;
    delay(CLOCK_PERIOD_MS);
  } while(1);
}//loopzinho
 
//Testes
//Ana
//ඞ𓆏ß𐐘
