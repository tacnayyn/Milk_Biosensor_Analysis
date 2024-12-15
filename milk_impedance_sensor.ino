/*
ad5933-test
    Reads impedance values from the AD5933 over I2C and prints them serially.
*/

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (50) //The initial frequency at which to start the impedance measurement.
#define FREQ_INCR   (100) //This is how much the frequency increases with each step.
#define NUM_INCR    (20) //This is the total number of frequency points in the sweep. 
#define REF_RESIST  (10000) // Reference resistor value in ohms for calibration

double gain[NUM_INCR+1];
int phase[NUM_INCR+1];

// To change the number of iterations, set the numIterations to any value that you want
const int numIterations = 1; //changed infinte sweeps if set to true
int loopCounter = 1; // Counter to track iterations
bool stopLoop = false;


void setup(void)
{
  // Begin I2C
  Wire.begin(); // AD9533 address 0x0D, registers it as a slave device in the I2C bus

  // Begin serial at 9600 baud for output
  Serial.begin(9600);
  Serial.println("AD5933 Config Started");
  // Perform initial configuration. Fail if any one of these fail.

 
  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(START_FREQ) &&
        AD5933::setIncrementFrequency(FREQ_INCR) &&
        AD5933::setNumberIncrements(NUM_INCR) &&
        AD5933::setPGAGain(PGA_GAIN_X1)))
        {
            Serial.println("FAILED in initialization!");
            while (true) ;
        }
/* Debugging
  if (!(AD5933::reset())) {
      Serial.println("FAILED to reset");
      while (true) ;

  } else if (!(AD5933::setInternalClock(true))) {
      Serial.println("FAILED to set clock");
      while (true) ;

  } else if (!(AD5933::setStartFrequency(START_FREQ))) {
      Serial.println("FAILED to set start freq");
      while (true);

  } else if (!(AD5933::setIncrementFrequency(FREQ_INCR))) {
      Serial.println("FAILED to set freq increment");
      while (true) ;

  } else if (!(AD5933::setNumberIncrements(NUM_INCR))) {
      Serial.println("FAILED to set num increments");
      while (true) ;

  } else if (!(AD5933::setPGAGain(PGA_GAIN_X1))) {
      Serial.println("FAILED to set gain");
      while (true) ;
    
  } else {
      //Serial.println("Passed initialization check");
  }
   */
    
  //Perform calibration sweep
  Serial.println("Start calibration sweep");
  if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1))
    Serial.println("----------------------------------Calibrated!");
  else
    Serial.println("----------------------------------Calibration failed...");

}

void loop(void)
{

     if (!stopLoop) {
      
        Serial.println("----------------------------------Iteration: "+ String(loopCounter));

        // Easy to use method for frequency sweep
        Serial.println("----------------------------------FS - Easy Begin----------------------------------");
        frequencySweepEasy();
        Serial.println("----------------------------------FS  -  Easy End----------------------------------");


        // Delay
        delay(3000);


        // Complex but more robust method for frequency sweep
        // Serial.println("----------------------------------FS - Raw Begin----------------------------------");
        // frequencySweepRaw();
        // Serial.println("----------------------------------FS  -  Raw End----------------------------------");

        // Delay
        // delay(5000);

        
        loopCounter++;

        if (loopCounter > numIterations) {
          stopLoop = true; // Set flag to true to stop the loop
        }
      } else {
      
          Serial.println("----------------------------------Finished executing the loop.");
          while (true);
      }
  
    
}

// Easy way to do a frequency sweep. Does an entire frequency sweep at once and
// stores the data into arrays for processing afterwards. This is easy-to-use,
// but doesn't allow you to process data in real time.
void frequencySweepEasy() {
    // Create arrays to hold the data
    int real[NUM_INCR+1], imag[NUM_INCR+1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR+1)) {
      // Print the frequency data
      int cfreq = START_FREQ/1000;
      for (int i = 0; i < NUM_INCR+1; i++, cfreq += FREQ_INCR/1000) {

        // Compute impedance
        double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        double impedance = 1/(magnitude*gain[i]);

        // Print raw frequency data
        /*
        Serial.print(cfreq);
        Serial.print(": R=");
        Serial.print(real[i]);
        Serial.print("/I=");
        Serial.print(imag[i]);
        Serial.print("  |Z|=");
        Serial.println(impedance);
        */

        Serial.print(String(loopCounter));
        Serial.print(",frequency_sweep_easy");
        Serial.print("," + String(cfreq));
        Serial.print("," + String(real[i]));
        Serial.print("," + String(imag[i]));
        Serial.print("," + String(gain[i]));
        Serial.print("," + String(magnitude));
        Serial.println("," + String(impedance));
      }
      Serial.println("Frequency sweep complete!");
    } else {
      Serial.println("Frequency sweep failed...");
    }
}



// Removes the frequencySweep abstraction from above. This saves memory and
// allows for data to be processed in real time. However, it's more complex.
void frequencySweepRaw() {
    // Create variables to hold the impedance data and track frequency
    int real, imag, i = 0, cfreq = START_FREQ/1000;

    // Initialize the frequency sweep
    if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
         {
             Serial.println("Could not initialize frequency sweep...");
         }

    // Perform the actual sweep
    while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Get the frequency data for this frequency point
        if (!AD5933::getComplexData(&real, &imag)) {
            Serial.println("Could not get raw frequency data...");
        }

        // Compute impedance
        double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
        double impedance = 1/(magnitude*gain[i]);

        // Print out the frequency data
        /*
        Serial.print(cfreq);
        Serial.print(": R=");
        Serial.print(real);
        Serial.print("/I=");
        Serial.print(imag);
        Serial.print("  |Z|=");
        Serial.println(impedance);
        */

        Serial.print(String(loopCounter));
        Serial.print(",frequency_sweep_raw");
        Serial.print("," + String(cfreq));
        Serial.print("," + String(real));
        Serial.print("," + String(imag));
        Serial.print("," + String(gain[i]));
        Serial.print("," + String(magnitude));
        Serial.println("," + String(impedance));

        // Increment the frequency
        i++;
        cfreq += FREQ_INCR/1000;
        AD5933::setControlMode(CTRL_INCREMENT_FREQ);
    }

    Serial.println("Frequency sweep complete!");

    // Set AD5933 power mode to standby when finished
    if (!AD5933::setPowerMode(POWER_STANDBY))
        Serial.println("Could not set to standby...");
}

