#define LED_PIN 12
#define CLOCK_PERIOD_MS 500

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}//setup

void bin_char_funcao(uint8_t palavrinha, uint8_t *bin_char){
  // Converte o código ascii para um vetor de binários
  uint8_t i = 0;

  do{
    bin_char[i] = (uint8_t)(palavrinha % 2);
    palavrinha /= 2;
    i++;
  }while (palavrinha > 0);
  
  // Se o binário tiver menos de 8 bits preenche o restante com bits 0
  do{
    bin_char[i] = 0;
    i++;
  }while (i < 8);

  // Inverte o vetor pra retornar
  for (uint8_t i = 0; i < 4; i++){
    uint8_t aux = bin_char[i];
    bin_char[i] = bin_char[7 - i];
    bin_char[7 - i] = aux;
  }//for
}//bin_char_funcao

void get_checksum(uint8_t *bin_char, uint8_t *checksum){
  // Gera o CRC
  // Código CRC utilizado
  uint8_t code[] = {1, 1, 0, 1};
  uint8_t tamanhozinho_code = sizeof(code) / sizeof(code[0]);

  // Passa o binário para um vetor que irá gerar o CRC
  // Como o vetor de CRC tem o binário + o código, preenche os vazios com zero
  uint8_t auxzinho[8 + tamanhozinho_code - 1];
  uint8_t tamanhozinho_auxzinho = sizeof(auxzinho) / sizeof(auxzinho[0]);
  for (uint8_t i = 0; i < tamanhozinho_auxzinho; i++){
    if (i >= 8) auxzinho[i] = 0;
    else auxzinho[i] = bin_char[i];
  }//for

  // Faz a operação bit por bit
  // Começa no bit 0 e vai até o bit 7
  // Realiza operação XOR (bits iguais 0, bits diferentes 1) entre o binário do caractere
  // Código CRC
  for (uint8_t i = 0; i <= tamanhozinho_auxzinho - tamanhozinho_code;){
    for (uint8_t j = 0; j < tamanhozinho_code; j++) auxzinho[i + j] = auxzinho[i + j] == code[j] ? 0 : 1;
    
    for (i; i < tamanhozinho_auxzinho && auxzinho[i] != 1; i++);
  
  }//for

  // Pega os 3 últimos bits para retornar
  int i=0;
  do{
    checksum[i] = auxzinho[tamanhozinho_auxzinho - 3 + i];
    i++;
  }while(i < 3);
}//for

void NRZ_L(int tamarindo_msgzinha, uint8_t *bin_msg[]){
  // Codificador NRZ-L
  for (uint8_t i = 0; i < tamarindo_msgzinha; i++){
    // Envia 2 sinais para avisar que será enviado um fragmento
    digitalWrite(LED_PIN, HIGH);
    delay(CLOCK_PERIOD_MS);
    digitalWrite(LED_PIN, HIGH);
    delay(CLOCK_PERIOD_MS);

    // Se o bit for 0, liga o led (codigo 1), se o bit for 1, apaga o led (codigo -1)
    for (uint8_t j = 0; j < 11; j++){
      if (bin_msg[i][j] != 0){
        Serial.print(-1);
        Serial.print("\t");
        digitalWrite(LED_PIN, LOW);
      }else{
       Serial.print(1);
        Serial.print("\t");
        digitalWrite(LED_PIN, HIGH);
      }//else
      delay(CLOCK_PERIOD_MS);
    }//for
    Serial.println();
  }//for

  // Apaga o led no final da transmissão
  digitalWrite(LED_PIN, LOW);
  delay(CLOCK_PERIOD_MS);
}//NRZ_L

void NRZ_I(int tamarindo_msgzinha, uint8_t *bin_msg[]){
  // Codificador NRZ-I
  int8_t sinalzinho;
  for (uint8_t i = 0; i < tamarindo_msgzinha; i++){
    // Envia 2 sinais para avisar que será enviado um fragmento
    digitalWrite(LED_PIN, HIGH);
    delay(CLOCK_PERIOD_MS);
    digitalWrite(LED_PIN, HIGH);
    delay(CLOCK_PERIOD_MS);

    for (uint8_t j = 0; j < 11; j++){
      // Se for o primeiro bit trasmitido, faz como no NRZ-L mas salva o valor antigo
      if (j == 0){
        if (bin_msg[i][j] != 0){
          Serial.print(-1);
          Serial.print("\t");
          digitalWrite(LED_PIN, LOW);
          sinalzinho = -1;
        } else{
          Serial.print(1);
          Serial.print("\t");
          digitalWrite(LED_PIN, HIGH);
          sinalzinho = 1;
        }//else
      }//for

      // Senao, quando vem um bit zero mantem o mesmo nível mas quando vem um bit 1 
      // inverte o nível
      else{
        if (bin_msg[i][j] == 0) {
          Serial.print(sinalzinho);
          Serial.print("\t");
        } else {
          if (sinalzinho != -1){
            Serial.print(-1);
            Serial.print("\t");
            digitalWrite(LED_PIN, LOW);
            sinalzinho = -1;
          } else{
            Serial.print(1);
            Serial.print("\t");
            digitalWrite(LED_PIN, HIGH);
            sinalzinho = 1;
          }//else
        }//else
      }//else
      delay(CLOCK_PERIOD_MS);
    }//for
    Serial.println();
  }//for
  
  // Apaga o led no final da transmissão
  digitalWrite(LED_PIN, LOW);
  delay(CLOCK_PERIOD_MS);
}//NRZ_I

void loop() {
  if (Serial.available() > 0) {
    // Pega a mensagem (string) digitada no serial
    String mensagenzinha = Serial.readString();
    // Remove \n, espaços no começo e no final...
    mensagenzinha.trim();
    // Quantidade de caracteres da mensagem
    int tamarindo_msgzinha = mensagenzinha.length();

    Serial.println("Mensagem a ser transmitida: ");
    Serial.println(mensagenzinha);
    Serial.println();
    Serial.println("Codificação binária: ");

    // Cria um vetor para armazenar os binários dos caracteres
    uint8_t *bin_msg[tamarindo_msgzinha];

    // Para cada caractere faz o processamento
    for (uint8_t i = 0; i < tamarindo_msgzinha; i++) {
      // Pega o código ascii do caractere
      uint8_t ascii = mensagenzinha.charAt(i);

      // Transforma pra binário
      uint8_t *bin_char = (uint8_t *)malloc(sizeof(uint8_t) * 8);
      bin_char_funcao(ascii, bin_char);

      // Gera o checksum
      uint8_t *char_checksum = (uint8_t *)malloc(sizeof(uint8_t) * 3);
      get_checksum(bin_char, char_checksum);

      // Cria um vetor com a palavra binária + código CRC
      uint8_t *palavrinha_completa = (uint8_t *)malloc(sizeof(uint8_t) * 11);
      uint8_t j = 0;
      for (uint8_t i = 0; i < 11; i++) {
        if (i >= 8) { 
          palavrinha_completa[i] = char_checksum[j];
          j++;
        }else palavrinha_completa[i] = bin_char[i];

      }//for

      for (int i = 0; i < 11; i++) Serial.print(palavrinha_completa[i]);
      Serial.println();

      // Adiciona a palavra completa no vetor para transmitir
      bin_msg[i] = palavrinha_completa;
    }//for

    // Transmite os dados
    Serial.println();
    Serial.println("Transmitindo...");
//auxiliar para escolher o NRZI OU NRZL
    int aux_NR = 0;
    if(aux_NR ==0){
    NRZ_I(tamarindo_msgzinha, bin_msg); 
    }else{//if
    NRZ_L(tamarindo_msgzinha, bin_msg);
    }//else   
    Serial.println("Transmissão finalizada");
  }//if
}//loop

//Testes
//Ana
//ඞ𓆏ß
