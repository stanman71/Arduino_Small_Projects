const int echoPin = 4;  //D2
const int trigPin = 0;  //D3

 
long duration, distance; 
 
void setup()
{
    Serial.begin (115200);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}
 
void loop()
{
    int i = 0;
    
    while (i < 25) {

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        
        duration = duration + pulseIn(echoPin, HIGH);

        i = i + 1;

        delay(50);
      
    }

    distance = duration/1455;    //averge value (25x), base 58.2
    duration = 0;
    
    Serial.println(distance);
    delay(50);
}
