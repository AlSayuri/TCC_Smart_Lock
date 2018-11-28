// Bibliotecas
  // Biblioteca Json disponível em: arduinojson.org
  #include <ArduinoJson.h> 
  // Biblioteca para conectar à Internet
  #include <Ethernet.h> 
  // Biblioteca para se comunicar com dispositivos 
  //periféricos ou entre microcontroladores
  #include <SPI.h> 
  // Biblioteca do motor Micro Servo motor SG90
  #include <Servo.h>
// Fim Bibiotecas

// Variáveis
  // Variáveis Ethernet
    EthernetServer server(60);
    EthernetClient client;
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  // Fim Variáveis Ethernet
  // Variáveis Tranca
    int porta_status;//1 = Aberta; 0 = Fechada
  // Fim Variáveis Tranca
  // Variáveis Motor
     Servo motor;
  // Fim Variáveis Motor
//Fim Variáveis

void setup() {
  //Inicializar porta serial
  Serial.begin(9600);

  // Liga o motor no pino 7 ao objeto servo
  motor.attach(7);  

  //Inicializar a Ethernet
  Ethernet.begin(mac);
  server.begin();
  Serial.println("Conectando à Ethernet");

  //Inicializar com a porta fechada
  porta_status = 0;
  
}

void loop() {
  //Conectar ao servidor HTTP
  client.setTimeout(10000);
  if (!client.connect("tcc-smartlock.herokuapp.com", 80)) {
    Serial.println(F("Conexão falhou"));
    return;
  }

  Serial.println(F("Conectado ao Servidor!"));

  //Enviar solicitação HTTP
  client.println(F("GET /twitter/nome_suario HTTP/1.0")); //Colocar nome do usuário sem a '#'
  client.println(F("Host: tcc-smartlock.herokuapp.com"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Falha ao enviar solicitação"));
    return;
  }

  //Verificar o status do HTTP
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Resposta inesperada: "));
    Serial.println(status);
    return;
  }

  //Ignorar cabeçalhos HTTP
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Resposta inválida"));
    return;
  }

  //Alocar o JsonBuffer
  //Usar arduinojson.org/assistant para calcular a capacidade.
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 35;
  DynamicJsonBuffer jsonBuffer(capacity);

  //Analisar objeto JSON
  JsonObject& root = jsonBuffer.parseObject(client);
  String hashtag = root["tweets"]["text"]; // Pega a hastag
    
  Serial.println("Resposta:");
  Serial.println(hashtag);

  //Desconectar
  client.stop();

  //---------(✿◠‿◠)-------------٩(｡•́‿•̀｡)۶-----------＼（＾○＾）人（＾○＾）／---------
  //Abrir ou fechar a tranca
  if (hashtag.equalsIgnoreCase("abrir") && porta_status == 0){ //Abrir porta
    Serial.println("Abrindo porta");
    porta_status = 1;
    motor.write(90); // Motor gira 90º
    delay(20); // Espera de 20ms para que o motor atinja a posição
  }
  else if (hashtag.equalsIgnoreCase("abrir") && porta_status == 1){ //Manter porta aberta
    Serial.println("Porta já aberta");
  }
  else if (hashtag.equalsIgnoreCase("fechar") && porta_status == 1){ //Fechar porta
    Serial.println("Fechando porta");
    porta_status = 0;
    motor.write(-90); // Motor gira -90º
    delay(20); // Espera de 20ms para que o motor atinja a posição
  }
  else if(hashtag.equalsIgnoreCase("fechar") && porta_status == 0){ //Manter porta fechada
    Serial.println("Porta já fechada");
  }

  delay(5000); // Espera 5 segundos para iniciar novo ciclo
}
