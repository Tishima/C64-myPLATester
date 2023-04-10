// based on the work from AC Lovell - ACs 8-Bit Zone 10/2020
// https://www.youtube.com/watch?v=Yru8bx_IqN8
//
// some little improvements for the PLA Tester shield by Bjoern Buettner 04/2023


const bool VERBOSE = false;

uint8_t   outputVec;    // PLA outputs
uint8_t   expectedVec;  // expected PLA outputs, calculated
uint16_t  inputVec;     // PLA inputs
uint16_t  shiftReg;

char    outbuffer[120];
char    inbuffer[120];
//String  str;
//char    ch;

const uint16_t MASK[32] = {
  0x6734, 0x2734, 0x273c, 0x37b4,
  0x57b4, 0x37bc, 0x0827, 0x082f, 
  0x87b0, 0x37f4, 0x37b4, 0x57f4, 
  0x57b4, 0x37fc, 0x37bc, 0x57fc, 
  0x57bc, 0x07fc, 0x07bc, 0x6738, 
  0x072c, 0x273c, 0x072c, 0x002f, 
  0x068c, 0x070c, 0x060c, 0x070c, 
  0x078c, 0x8000, 0x8000, 0x87b0
};

const uint16_t COMP[32] = {
  0x6514, 0x2714, 0x2710, 0x2694,
  0x4694, 0x2690, 0x0825, 0x0821, 
  0x8680, 0x36d4, 0x3684, 0x56d4, 
  0x5684, 0x36d0, 0x3680, 0x56d0, 
  0x5680, 0x06d8, 0x0688, 0x6410, 
  0x0408, 0x2510, 0x0708, 0x002b, 
  0x0088, 0x0108, 0x0208, 0x0508, 
  0x0608, 0x0000, 0x8000, 0x0680
};

bool pterm[32];
long total;
long passes;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.print("Initializing test pins.\n");

  inputVec = 0xFF;

  // PLA inputs are Arduino outputs
  // We need 16 output pins
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);

  pinMode(38, OUTPUT);     //error LED
  pinMode(39, OUTPUT);     //passe LED

  digitalWrite(38, HIGH ); // error led off
  digitalWrite(39, HIGH ); // pass led off

  digitalWrite(22, HIGH ); // 
  digitalWrite(23, HIGH ); // 
  digitalWrite(24, HIGH ); // 
  digitalWrite(25, HIGH ); // 
  digitalWrite(26, HIGH ); // 
  digitalWrite(27, HIGH ); // 
  digitalWrite(28, HIGH ); // 
  digitalWrite(29, HIGH ); // 
  digitalWrite(30, HIGH ); // 
  digitalWrite(31, HIGH ); // 
  digitalWrite(32, HIGH ); // 
  digitalWrite(33, HIGH ); // 
  digitalWrite(34, HIGH ); // 
  digitalWrite(35, HIGH ); // 
  digitalWrite(36, HIGH ); // 
  digitalWrite(37, HIGH ); // 

  // PLA outputs are Arduino inputs
  // We need 8 inputs
  outputVec = 0x00;
  pinMode(44, INPUT);
  pinMode(45, INPUT);
  pinMode(46, INPUT);
  pinMode(47, INPUT);
  pinMode(48, INPUT);
  pinMode(49, INPUT);
  pinMode(50, INPUT);
  pinMode(51, INPUT);

  passes = 0;
  total  = 0;
}


void loop() {
  // put your main code here, to run repeatedly:
  int i;
  long v; // needs to reach 65536 without rolling over

  Serial.print("Starting test. This can take up to 1 minute.\n");

  // Loop through 2^16 possible input combinations. 
  // Calculate the product terms.
  for (inputVec=0,v=0; v<65536; v++,inputVec++) {
    shiftReg = inputVec;
    // Transfer values to the pin signals D37:D22
    for (i=0; i<16; i++) {
      if (shiftReg & 0x0001) {
        digitalWrite(22+i, HIGH);
      }
      else {
        digitalWrite(22+i, LOW);
      }
      shiftReg = shiftReg >> 1;
    }
  
    // Calculate the Product terms for the set of inputVec values
    for (i=0; i<32; i++) {
      pterm[i] = ( inputVec & MASK[i] ) == COMP[i];
    }

    // Calculate the expectedVec
    expectedVec = 0x7F;
    if ( pterm[0] ) {
        expectedVec = expectedVec & 0xBF;  // clear bit 6
        expectedVec = expectedVec | 0x80;  // set bit 7
    }
      
    if ( pterm[1] || pterm[2] ) {
        expectedVec = expectedVec & 0xDF;  // clear bit 5
        expectedVec = expectedVec | 0x80;  // set bit 7
    }
      
    if ( pterm[3] || pterm[4] || pterm[5] || pterm[6] || pterm[7] ) {
        expectedVec = expectedVec & 0xEF;  // clear bit 4
        expectedVec = expectedVec | 0x80;  // set bit 7
    }
                                    
    if ( pterm[31] )
        expectedVec = expectedVec & 0xF7;  // clear bit 3
    
    if ( pterm[9] || pterm[10] || pterm[11] || 
         pterm[12] || pterm[13] || pterm[14] || 
         pterm[15] || pterm[16] || pterm[17] || pterm[18] 
    ) {
        expectedVec = expectedVec & 0xFB;  // clear bit 2
        expectedVec = expectedVec | 0x80;  // set bit 7
    }
                                    
    if ( pterm[19] || pterm[20] ) {
        expectedVec = expectedVec & 0xFD;  // clear bit 1
        expectedVec = expectedVec | 0x80;  // set bit 7
    }

    if ( pterm[21] || pterm[22] || pterm[23] ) {
        expectedVec = expectedVec & 0xFE;  // clear bit 0
        expectedVec = expectedVec | 0x80;  // set bit 7
    }
    
    if ( pterm[24] || pterm[25] || pterm[26] || 
         pterm[27] || pterm[28] || pterm[30]
    )
        expectedVec = expectedVec | 0x80;

    // Read the DUT outputs and store in outputVec.
    //Serial.print(" outputs=");
    outputVec = 0x00;
    for (i=7; i>=0; i--) {
      outputVec = outputVec << 1;
      if (digitalRead(44+i)) {
        outputVec = outputVec | 0x01;
      }
    }

    // Compare outputVec to expectedVec
    if (VERBOSE) 
    {
      sprintf(outbuffer, "inputs=%04X outputs=%02X expected=%02X\n",
                          inputVec,outputVec,expectedVec);
      Serial.print(outbuffer);
    }
    
    total=total+1;
    if ( outputVec != expectedVec ) {
      sprintf(outbuffer, "ERROR: inputs=%04X outputs=%02X expected=%02X\n",
                          inputVec,outputVec,expectedVec);
      Serial.print(outbuffer);
     break; // NOTE: comment this line to continue testing after failures
    }
    else 
      passes=passes+1;
  }

  // Done !
  Serial.println("The test is complete.");
  sprintf(outbuffer, "There were %ld passing vectors out of %ld total.", passes, total);
  Serial.println(outbuffer);

  // check passes and turn the leds on
   if (passes != 65536) {
      digitalWrite(38, LOW); // error LED on
  }
    else 
   {
      digitalWrite(39, LOW); // pass LED on
   }

  
  while(1);
}
