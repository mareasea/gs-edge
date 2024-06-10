 // Incluindo as bibliotecas necessárias para o projeto
#include <DHT.h> // Biblioteca para o sensor de temperatura e umidade DHT
#include <LiquidCrystal.h>  // Biblioteca para LCD I2C
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <EEPROM.h> // Biblioteca para a EEPROM

// Declaração das constantes
#define DHTPIN 7 // Pino do sensor DHT
#define DHTTYPE DHT22 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE); // Inicializa o sensor DHT

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Inicializa o display LCD
RTC_DS1307 RTC; // Inicializa o RTC

#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 1  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3

// Configurações da EEPROM
const int maxRecords = 100; // Número máximo de registros na EEPROM
const int recordSize = 14; // Tamanho de cada registro em bytes
int startAddress = 0; // Endereço inicial na EEPROM
int endAddress = maxRecords * recordSize; // Endereço final na EEPROM
int currentAddress = 0; // Endereço atual na EEPROM

// Declaração das variáveis
int ldr = A0; // Pino de entrada analógica conectado ao LDR
int sensorPh = A1; // Pino do sensor de pH
int sensorUv = A2; // Pino do sensor UV
int ledVermelho = 10; // Pino do LED vermelho
int ledAmarelo = 9; // Pino do LED amarelo
int ledVerde = 8; // Pino do LED verde
float temp; // Variável para armazenar a temperatura
int valorPh; // Variável para armazenar o valor do pH
int valorUv; // Variável para armazenar o valor UV
int UV_index = 0; // Índice UV

// Array símbolo de grau
byte grau[8] ={ 
    B00001100,
    B00010010,
    B00010010,
    B00001100,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
};

// Arrays para desenho de um peixe no LCD
byte raboCimaPeixe[] = {   
    B00000,
    B00000,
    B00001,
    B00000,
    B01000,
    B01100,
    B01110,
    B01111
};
byte raboBaixoPeixe[] = {
    B01111,
    B01110,
    B01100,
    B01000,
    B00000,
    B00000,
    B00000,
    B00000
};
byte corpoCimaPeixe[] = {
    B00000,
    B00000,
    B10000,
    B11100,
    B11011,
    B01101,
    B11011,
    B10111
};
byte corpoBaixoPeixe[] = {
    B11111,
    B00111,
    B00011,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
};
byte cabecaCimaPeixe[] = {
    B00000,
    B00110,
    B01001,
    B01001,
    B00110,
    B10000,
    B01000,
    B11100
};
byte cabecaBaixoPeixe[] = {
    B11000,
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
};

void setup() {
    Serial.begin(9600); // Inicia a comunicação serial
    pinMode(ledVermelho, OUTPUT); // Configura o pino do LED vermelho como saída
    pinMode(ledAmarelo, OUTPUT); // Configura o pino do LED amarelo como saída
    pinMode(ledVerde, OUTPUT); // Configura o pino do LED verde como saída
    pinMode(sensorPh, INPUT); // Configura o pino do sensor de pH como entrada
    pinMode(sensorUv, INPUT); // Configura o pino do sensor UV como entrada
    dht.begin(); // Inicia o sensor DHT
    lcd.begin(16, 2); // Inicializa o LCD com 16 colunas e 2 linhas
    lcd.createChar(0, grau); // Cria o caractere personalizado para o símbolo de grau
    RTC.begin(); // Inicia o RTC
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); // Ajusta o RTC com a data e hora de compilação
    inicializacao(); // Função para inicializar o display LCD
    lcd.createChar(0, grau); // Cria novamente o caractere personalizado para o símbolo de grau
    EEPROM.begin(); // Inicia a EEPROM
}

void loop() {
    float somaLux = 0; // Soma dos valores de lux
    float somaTemp = 0; // Soma dos valores de temperatura
    float somaPhLevel = 0; // Soma dos valores de pH
    float somaUv = 0; // Soma dos valores UV

    // Realiza 10 leituras para calcular a média
    for (int i = 0; i < 10; ++i) {
        int valorLdr = analogRead(ldr); // Ler o valor analógico do pino A0
        int valorLdrConvert = map(valorLdr, 1015, 8, 8, 1015); // Converte o valor lido para uma escala ajustada
        float Vout =  valorLdrConvert * (5.0 / 1023.0); // Converte a leitura analógica para tensão

        // Garantir que Vout não seja zero para evitar divisão por zero
        if (Vout != 0) {
            int R1 = 10000; // Resistor fixo de 10k ohms
            float R_LDR = R1 * (5.0 / Vout - 1.0); // Calcular resistência do LDR

            // Assumindo que a relação resistência-lux é inversa e ajustando os coeficientes
            if (R_LDR != 0) {
                float lux = pow((R1 / R_LDR), 1.25) * 100; // Exemplo de ajuste, pode variar conforme seu LDR
                somaLux += lux; // Soma o valor de lux
            }
        }

        temp = dht.readTemperature(); // Ler a temperatura do sensor DHT
        somaTemp += temp; // Soma o valor da temperatura

        valorPh = analogRead(sensorPh); // Ler valor do potenciômetro (simulando sensor de pH)
        float voltagemPh = valorPh * (5.0 / 1023.0); // Converter os valores para as unidades desejadas
        float phLevel = voltagemPh * 2.8; // Ajustar a escala para 0-14 de pH
        somaPhLevel += phLevel; // Soma o valor do pH

        valorUv = analogRead(sensorUv); // Ler o valor do sensor UV
        somaUv += valorUv; // Soma o valor UV

        delay(200);
    }

    // Calcular as médias
    float mediaLux = somaLux / 10;
    Serial.print("Índice de luminosidade (em lux): ");
    Serial.print(mediaLux);
    String luxMessage = validarLux(mediaLux); // Validar o valor de lux

    float mediaTemp = somaTemp / 10;
    String tempMessage = avaliarTemperatura(mediaTemp); // Avaliar a temperatura

    float mediaPhLevel = somaPhLevel / 10;
    String phMessage = avaliarPh(mediaPhLevel); // Avaliar o pH

    float mediaUv = somaUv / 10;
    int indiceUV = calcularNivelUv(mediaUv); // Calcular o índice UV
    String uvMessage = avaliarUv(indiceUV); // Avaliar o índice UV

    registrarLog(mediaLux, mediaTemp, mediaPhLevel, indiceUV); // Registrar os dados no log

    mostrarInfo(mediaLux, luxMessage, mediaTemp, tempMessage, mediaPhLevel, phMessage, mediaUv, indiceUV, uvMessage); // Mostrar as informações no LCD

    delay(8000); // Aguardar 8 segundos antes da próxima iteração 
}

// Função para apagar todos os LEDs
void apagarLeds() {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, LOW);
}

// Função para validar o nível de lux
String validarLux(float mediaLux) {
  String luxMessage = ""; // Inicializa a mensagem de lux como uma string vazia

  // Verifica se o nível de lux está fora dos valores ideais
  if (mediaLux < 5000 || mediaLux > 15000) {
    apagarLeds(); // Apaga todos os LEDs
    digitalWrite(ledVermelho, HIGH); // Acende o LED vermelho
    luxMessage = "(Nivel critico!)"; // Define a mensagem de lux como "Nível crítico"
    Serial.print("\t");
    Serial.println(luxMessage); // Imprime a mensagem de lux no monitor serial

  // Verifica se o nível de lux está acima de 10000
  } else if (mediaLux > 10000) {
    apagarLeds(); // Apaga todos os LEDs
    digitalWrite(ledAmarelo, HIGH); // Acende o LED amarelo
    luxMessage = "(Nível de alerta!)"; // Define a mensagem de lux como "Nível de alerta"
    Serial.print("\t");
    Serial.println(luxMessage); // Imprime a mensagem de lux no monitor serial

  // Caso contrário, o nível de lux está dentro do intervalo ideal
  } else {
    apagarLeds(); // Apaga todos os LEDs
    digitalWrite(ledVerde, HIGH); // Acende o LED verde
    luxMessage = "(Nível ideal!)"; // Define a mensagem de lux como "Nível ideal"
    Serial.print("\t");
    Serial.println(luxMessage); // Imprime a mensagem de lux no monitor serial
  }

  return luxMessage; // Retorna a mensagem de lux
}

// Função para avaliar a temperatura
String avaliarTemperatura(float mediaTemp) {
  String tempMessage = ""; // Inicializa a mensagem de temperatura como uma string vazia

  Serial.print("Temperatura: "); // Imprime "Temperatura: " no monitor serial
  Serial.print(mediaTemp); // Imprime o valor da temperatura no monitor serial
  Serial.print("°C"); // Imprime "°C" no monitor serial

  // Verifica se a temperatura está dentro do intervalo ideal
  if (mediaTemp >= 23 && mediaTemp <= 28) {
    tempMessage = "Temp. ideal"; // Define a mensagem de temperatura como "Temp. ideal"
  } else {
    tempMessage = "Alerta de temp."; // Define a mensagem de temperatura como "Alerta de temp."
  }

  Serial.print("\t");
  Serial.println(tempMessage); // Imprime a mensagem de temperatura no monitor serial

  return tempMessage; // Retorna a mensagem de temperatura
}

// Função para avaliar o nível de pH
String avaliarPh(float mediaPhLevel) {
  String phMessage = ""; // Inicializa a mensagem de pH como uma string vazia

  Serial.print("pH: "); // Imprime "pH: " no monitor serial
  Serial.print(mediaPhLevel); // Imprime o valor do pH no monitor serial

  // Verifica se o nível de pH está dentro do intervalo ideal para corais
  if (mediaPhLevel >= 8.0 && mediaPhLevel <= 8.4) {
    phMessage = "Nível Ideal"; // Define a mensagem de pH como "Ideal para Corais"
  } else {
    phMessage = "Fora do ideal!"; // Define a mensagem de pH como "Fora do ideal!"
  }

  Serial.print("\t");
  Serial.println(phMessage); // Imprime a mensagem de pH no monitor serial

  return phMessage; // Retorna a mensagem de pH
}

// Função para avaliar o índice UV
String avaliarUv(int indiceUV) {
  String uvMessage = ""; // Inicializa a mensagem de UV como uma string vazia

  Serial.print("Índice UV: "); // Imprime "Índice UV: " no monitor serial
  Serial.print(indiceUV); // Imprime o valor do índice UV no monitor serial
  // Verifica se o índice UV está acima do valor aceitável
  if (indiceUV > 2) {
    uvMessage = "Crítico!"; // Define a mensagem de UV como "Crítico!"
  } else {
    uvMessage = "Aceitável!"; // Define a mensagem de UV como "Aceitável!"
  }

  Serial.print("\t");
  Serial.println(uvMessage); // Imprime a mensagem de UV no monitor serial

  return uvMessage; // Retorna a mensagem de UV
}

// Função para calcular o nível de UV com base na tensão
int calcularNivelUv(float valorUv) {
  // Calcula a tensão em milivolts
  int tensao = (valorUv * (5.0 / 1023.0)) * 1000;

  // Compara a tensão com os valores da tabela UV_Index
  if (tensao > 0 && tensao < 50) {
    UV_index = 0;
  }
  else if (tensao > 50 && tensao <= 227) {
    UV_index = 0;
  }
  else if (tensao > 227 && tensao <= 318) {
    UV_index = 1;
  }
  else if (tensao > 318 && tensao <= 408) {
    UV_index = 2;
  }
  else if (tensao > 408 && tensao <= 503) {
    UV_index = 3;
  }
  else if (tensao > 503 && tensao <= 606) {
    UV_index = 4;
  }
  else if (tensao > 606 && tensao <= 696) {
    UV_index = 5;
  }
  else if (tensao > 696 && tensao <= 795) {
    UV_index = 6;
  }
  else if (tensao > 795 && tensao <= 881) {
    UV_index = 7;
  }
  else if (tensao > 881 && tensao <= 976) {
    UV_index = 8;
  }
  else if (tensao > 976 && tensao <= 1079) {
    UV_index = 9;
  }
  else if (tensao > 1079 && tensao <= 1170) {
    UV_index = 10;
  }
  else if (tensao > 1170) {
    UV_index = 11;
  }

  return UV_index; // Retorna o índice UV calculado
}

// Função de inicialização do LCD e exibição de uma animação de um peixe
void inicializacao() {
  // Define o caractere customizado para o rabo do peixe na linha superior
  lcd.createChar(7, raboCimaPeixe);
  lcd.setCursor(7, 0);
  lcd.write(7);

  // Define o caractere customizado para o rabo do peixe na linha inferior
  lcd.createChar(8, raboBaixoPeixe);
  lcd.setCursor(7, 1);
  lcd.write(8);

  // Define o caractere customizado para o corpo do peixe na linha superior
  lcd.createChar(9, corpoCimaPeixe);
  lcd.setCursor(8, 0);
  lcd.write(9);

  // Define o caractere customizado para o corpo do peixe na linha inferior
  lcd.createChar(12, corpoBaixoPeixe);
  lcd.setCursor(8, 1);
  lcd.write(12);

  // Define o caractere customizado para a cabeça do peixe na linha superior
  lcd.createChar(13, cabecaCimaPeixe);
  lcd.setCursor(9, 0);
  lcd.write(13);

  // Define o caractere customizado para a cabeça do peixe na linha inferior
  lcd.createChar(14, cabecaBaixoPeixe);
  lcd.setCursor(9, 1);
  lcd.write(14);

  // Animação de rolagem para a esquerda
  for (int posicao = 0; posicao < 3; posicao++) {
    lcd.scrollDisplayLeft();
    delay(500);
  }

  // Animação de rolagem para a direita
  for (int posicao = 0; posicao < 6; posicao++) {
    lcd.scrollDisplayRight();
    delay(500);
  }

  delay(500);

  // Limpa a tela do LCD
  lcd.clear();
  // Posiciona o cursor na coluna 3, linha 0
  lcd.setCursor(3, 0);
  // Exibe o texto "MareaSea" na linha superior
  lcd.print("MareaSea");
  // Posiciona o cursor na coluna 3, linha 1
  lcd.setCursor(3, 1);
  // Exibe o texto "Blue Future" na linha inferior
  lcd.print("Blue Future");
  delay(5000); // Aguarda 5 segundos
  lcd.clear(); // Limpa a tela após 5 segundos
}

// Função para exibir informações no LCD
void mostrarInfo(float mediaLux, String luxMessage, float mediaTemp, String tempMessage, float mediaPhLevel, String phMessage, float mediaUv, int indiceUV, String uvMessage) {
  // Exibe a informação de lux
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lux: ");
  lcd.setCursor(7, 0);
  lcd.print(mediaLux, 1);
  lcd.setCursor(0, 1);
  lcd.print(luxMessage);
  delay(2000);

  // Exibe a informação de temperatura
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp : ");
  lcd.setCursor(7, 0);
  lcd.print(mediaTemp, 1);
  lcd.setCursor(11, 0);
  lcd.write((byte)0); // Mostra o símbolo do grau formado pelo array
  lcd.setCursor(0, 1);
  lcd.print(tempMessage);
  delay(2000);

  // Exibe a informação de pH
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH : ");
  lcd.setCursor(7, 0);
  lcd.print(mediaPhLevel, 1);
  lcd.setCursor(0, 1);
  lcd.print(phMessage);
  delay(2000);

  // Exibe a informação de UV
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UV: ");
  lcd.setCursor(4, 0);
  lcd.print(mediaUv);
  lcd.setCursor(0, 1);
  lcd.print("Escala: ");
  lcd.print(indiceUV);
}

// Função para obter o próximo endereço na EEPROM
void getNextAddress() {
  currentAddress += recordSize;
  if (currentAddress >= endAddress) {
    currentAddress = 0; // Volta para o começo se atingir o limite
  }
}

// Função para obter o log armazenado na EEPROM
void get_log() {
  Serial.println("Data stored in EEPROM:");
  Serial.println("Timestamp\tLux\tTemperature\tpH\tUV");

  // Loop para ler os dados armazenados na EEPROM
  for (int address = startAddress; address < endAddress; address += recordSize) {
    long timeStamp;
    int luxInt, tempInt, phInt, uvInt;

    // Ler dados da EEPROM
    EEPROM.get(address, timeStamp);
    EEPROM.get(address + 4, luxInt);
    EEPROM.get(address + 6, tempInt);
    EEPROM.get(address + 8, phInt);
    EEPROM.get(address + 10, uvInt);

    // Converter valores
    float lux = luxInt / 100.0;
    float temperature = tempInt / 100.0;
    float ph = phInt / 100.0;

    // Verificar se os dados são válidos antes de imprimir
    if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
      Serial.print(timeStamp);
      Serial.print("\t");
      Serial.print(lux);
      Serial.print("\t");
      Serial.print(temperature);
      Serial.print("\t\t");
      Serial.print(ph);
      Serial.print("\t");
      Serial.println(uvInt);
    }
  }
}

// Função para validar os valores e registrar no log
void registrarLog(float mediaLux, float mediaTemp, float mediaPhLevel, int indiceUV) {
  DateTime now = RTC.now();
  int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
  now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual
  DateTime adjustedTime = DateTime(now);

  // Verifica se algum valor está fora dos intervalos ideais
  if (mediaLux < 5000 || mediaLux > 15000 || mediaTemp < 23 || mediaTemp > 28 || mediaPhLevel < 8.0 || mediaPhLevel > 8.4 || indiceUV > 2) {
    // Registrar os dados na EEPROM
    int tempInt = (int)(mediaTemp * 100);
    int luxInt = (int)(mediaLux * 100);
    int phInt = (int)(mediaPhLevel * 100);

    EEPROM.put(currentAddress, now);
    EEPROM.put(currentAddress + 4, luxInt);
    EEPROM.put(currentAddress + 6, tempInt);
    EEPROM.put(currentAddress + 8, phInt);
    EEPROM.put(currentAddress + 10, indiceUV);

    getNextAddress(); // Atualiza o endereço atual na EEPROM

    // Imprimir os dados no terminal serial
    if (SERIAL_OPTION) {
      Serial.println("============================== ALERTA ==============================");
      Serial.print(adjustedTime.day());
      Serial.print("/");
      Serial.print(adjustedTime.month());
      Serial.print("/");
      Serial.print(adjustedTime.year());
      Serial.print(" ");
      Serial.print(adjustedTime.hour() < 10 ? "0" : ""); // Adiciona zero à esquerda se hora for menor que 10
      Serial.print(adjustedTime.hour());
      Serial.print(":");
      Serial.print(adjustedTime.minute() < 10 ? "0" : ""); // Adiciona zero à esquerda se minuto for menor que 10
      Serial.print(adjustedTime.minute());
      Serial.print(":");
      Serial.print(adjustedTime.second() < 10 ? "0" : ""); // Adiciona zero à esquerda se segundo for menor que 10
      Serial.print(adjustedTime.second());
      Serial.print(" - ");
      Serial.print("Lux: ");
      Serial.print(mediaLux);
      Serial.print(" Temp: ");
      Serial.print(mediaTemp);
      Serial.print(" pH: ");
      Serial.print(mediaPhLevel);
      Serial.print(" UV: ");
      Serial.println(indiceUV);
      Serial.println("====================================================================");
    }
  }
}
