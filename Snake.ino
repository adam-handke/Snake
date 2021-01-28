#include <LiquidCrystal.h>
#include <Keypad.h>

//upper snake in 8x5
byte up[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

//lower snake in 8x5
byte down[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111
};

//full snake in 8x5
byte full[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

//upper food in 8x5
byte food_up[] = {
  B00000,
  B00011,
  B11111,
  B01010,
  B00000,
  B00000,
  B00000,
  B00000
};

//lower food in 8x5
byte food_down[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B11111,
  B01010
};

//snake above food in 8x5
byte snake_up_food_down[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00011,
  B11111,
  B01010
};

//snake below food in 8x5
byte snake_down_food_up[] = {
  B00000,
  B00011,
  B11111,
  B01010,
  B11111,
  B11111,
  B11111,
  B11111
};

//keypad init
const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 3; //three columns (ignoring the 4th)
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte pin_rows[ROW_NUM] = {8, 7, 6, 5};
byte pin_column[COLUMN_NUM] = {4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

//LCD init
LiquidCrystal lcd(A4, A5, 9, 10, 11, A3);

//pins
const int pin_piezo = A0;

//game matrix (score excluded)
const int X = 14;
const int Y = 4;
int matrix[X][Y] = {{0}};

//max snake length (and win condition)
const int MAX_LENGTH = 42;
//snake position
int snakeX[MAX_LENGTH] = {0};
int snakeY[MAX_LENGTH] = {0};
int moveX = 0; //X movement in this moment
int moveY = 0; //Y movement in this moment

//state variables
char key;
int counter = 0;
int score = 0;
bool failure = false;

//displaying the game matrix
//empty	== 0
//snake	== 1
//food	== 2
void display(){
  for(int j=0; j<Y; j+=2){
    for(int i=0; i<X; i++){
      if(j == 0){
      	lcd.setCursor(i,0);
      }
      else{
        lcd.setCursor(i,1);
      }
      if(matrix[i][j] == 0 && matrix[i][j+1] == 0){
        lcd.print(' ');
      }
      else if(matrix[i][j] == 1 && matrix[i][j+1] == 0){
        lcd.write(byte(0));
      }
      else if(matrix[i][j] == 0 && matrix[i][j+1] == 1){
        lcd.write(byte(1));
      }
      else if(matrix[i][j] == 1 && matrix[i][j+1] == 1){
        lcd.write(byte(2));
      }
      else if(matrix[i][j] == 2 && matrix[i][j+1] == 0){
        lcd.write(byte(3));
      }
      else if(matrix[i][j] == 0 && matrix[i][j+1] == 2){
        lcd.write(byte(4));
      }
      else if(matrix[i][j] == 1 && matrix[i][j+1] == 2){
        lcd.write(byte(5));
      }
      else if(matrix[i][j] == 2 && matrix[i][j+1] == 1){
        lcd.write(byte(6));
      }
    }
  }
}

//displaying the score ("SC" + 2 digits)
void display_score(){
  char tmp;
  lcd.setCursor(14,0);
  lcd.print('S');
  lcd.setCursor(15,0);
  lcd.print('C');
  lcd.setCursor(14,1);
  tmp = '0' + ((score%100)/10);
  lcd.print(tmp);
  lcd.setCursor(15,1);
  tmp = '0' + ((score%10));
  lcd.print(tmp);
}

//generating 1 food in random place
void random_food(){
  int randX = 0;
  int randY = 0;
  //making sure that new food does not spawn on the snake
  do{
  	randX = random(0, X);
  	randY = random(0, Y);
  }while(matrix[randX][randY] != 0);
  matrix[randX][randY] = 2;
}

//resetting the game
void game_reset(){
  //cleaning up
  moveX = 0;
  moveY = 0;
  score = 0;
  failure = false;
  for(int j=0; j<Y; j++){
    for(int i=0; i<X; i++){
      matrix[i][j] = 0;
    }
  }
  for(int i=0; i<MAX_LENGTH; i++){
    snakeX[i] = -1;
    snakeY[i] = -1;
  }
  //initial snake position
  snakeX[0] = 1;
  snakeY[0] = 1;
  matrix[snakeX[0]][snakeY[0]] = 1;
  //initial movement is staying still
  moveX = 0;
  moveY = 0;
  //first food
  random_food();
  tone(pin_piezo, 156, 100);
  delay(250);
  tone(pin_piezo, 311, 100);
}

void make_move(){
  int nextX = snakeX[0] + moveX;
  int nextY = snakeY[0] + moveY;
  int tmpX, tmpY;
  
  if(nextX < 0 || nextX >= X || nextY < 0 || nextY >= Y || (matrix[nextX][nextY] == 1 && (moveX != 0 || moveY != 0))){
    //failure conditions fulfilled
    failure = true;
  }
  else{
    if(matrix[nextX][nextY] == 2){
      //eating
      tone(pin_piezo, 622, 30);
      score++;
      for(int i=0; i<=score; i++){
        matrix[nextX][nextY] = 1;
        tmpX = snakeX[i];
        tmpY = snakeY[i];
      	snakeX[i] = nextX;
      	snakeY[i] = nextY;
        nextX = tmpX;
        nextY = tmpY;
      }
      //1 new food
      random_food();
    }
    else{
      //regular movement
      for(int i=0; i<=score; i++){
        matrix[snakeX[i]][snakeY[i]] = 0;
        matrix[nextX][nextY] = 1;
        tmpX = snakeX[i];
        tmpY = snakeY[i];
      	snakeX[i] = nextX;
      	snakeY[i] = nextY;
        nextX = tmpX;
        nextY = tmpY;
      }      
    }
  }
}

void setup() {
  //debug serial output
  Serial.begin(9600);
  
  //piezo init
  pinMode(pin_piezo, OUTPUT);
  
  //lcd init
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  
  //loading graphics 8x5 for LCD
  lcd.createChar(0, up);
  lcd.createChar(1, down);
  lcd.createChar(2, full);
  lcd.createChar(3, food_up);
  lcd.createChar(4, food_down);
  lcd.createChar(5, snake_up_food_down);
  lcd.createChar(6, snake_down_food_up);
  
  //(re)setting the game
  game_reset();
}

//main logic loop
void loop() {
  display_score();
  display();
  counter = 0;
  
  //waiting for input for 0.5s, if nothing then auto move
  while( (key = keypad.getKey()) == NO_KEY && counter < 500 ){
    delay(1);
    counter++;
  }
  if(key == NO_KEY){
    key = 'M';
  }
  Serial.println(key); //DEBUG / MOVE LOG
  
  if(score >= MAX_LENGTH){
    //WIN
    tone(pin_piezo, 392, 200);
    delay(300);
    tone(pin_piezo, 330, 200);
    delay(300);
    tone(pin_piezo, 392, 200);
    delay(300);
    tone(pin_piezo, 330, 200);
    delay(300);
    tone(pin_piezo, 392, 200);
    
    while(key != '0'){
      key = keypad.waitForKey();
    };
    delay(300);
    game_reset();
  }
  else if(key == '0' || failure == true){
    //DEFEAT
    tone(pin_piezo, 233, 100);
    delay(200);
    tone(pin_piezo, 440, 100);
    delay(200);
    tone(pin_piezo, 233, 100);
    delay(200);
    tone(pin_piezo, 440, 100);
    delay(200);
    tone(pin_piezo, 233, 100);
    
    while(key != '0'){
      key = keypad.waitForKey();
    };
    delay(300);
    game_reset();
  }
  else if(key == '2'){
    //moving up
    tone(pin_piezo, 311, 30);
    moveX = 0;
    moveY = -1;
    make_move();
  }
  else if(key == '6'){
    //moving right
    tone(pin_piezo, 311, 30);
    moveX = 1;
    moveY = 0;
    make_move();
  }
  else if(key == '8'){
    //moving down
    tone(pin_piezo, 311, 30);
    moveX = 0;
    moveY = 1;
    make_move();
  }
  else if(key == '4'){
    //moving left
    tone(pin_piezo, 311, 30);
    moveX = -1;
    moveY = 0;
    make_move();
  }
  else if(key == 'M'){
    //auto move (nothing pressed)
    tone(pin_piezo, 156, 30);
    make_move();
  }
  else{
    //wrong key
    tone(pin_piezo, 156, 30);
  }
  delay(30);
}
 