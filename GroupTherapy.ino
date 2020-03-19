enum phases {WAIT, PLAY, RESULT};
enum personalities {INTROVERT, EXTROVERT};

byte phase = WAIT;
byte personality = INTROVERT;
bool success = false;

Timer phaseTimer;
#define WAIT_PHASE_TIME 4000
#define PLAY_PHASE_TIME 4000
#define RESULT_PHASE_TIME 5000

#define INTROVERT_HUE 195
#define INTROVERT_SAT 200
#define EXTROVERT_HUE 15
#define EXTROVERT_SAT 150

Timer flickerTimer;
#define FLICKER_DURATION 75
#define WAIT_PHASE_COLOR_CHANCE 500
#define INTROVERT_CHANCE 40//out of 100 chance of an extrovert

//Timer pulseTimer;
//#define PULSE_DURATION 500

void setup() {
  // put your setup code here, to run once:
  randomize();
  phaseTimer.set(WAIT_PHASE_TIME);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (phase) {
    case WAIT:
      waitLoop();
      break;
    case PLAY:
      playLoop();
      break;
    case RESULT:
      resultLoop();
      break;
  }

  byte signalData = (phase << 1) + (personality);
  setValueSentOnAllFaces(signalData);

  //temp display
  switch (phase) {
    case WAIT:
      waitDisplay();
      break;
    case PLAY:
      if (personality == INTROVERT) {
        setColor(makeColorHSB(INTROVERT_HUE, INTROVERT_SAT, 255));
      } else {//EXTROVERT
        setColor(makeColorHSB(EXTROVERT_HUE, EXTROVERT_SAT, 255));
      }
      break;
    case RESULT:
      resultDisplay();
      break;
  }
}

void waitLoop() {

  //determine neighborhood
  if (!isAlone()) { //only do all this stuff if you have neighbors at all
    byte extrovertNeighbors = 0;
    byte introvertNeighbors = 0;
    byte neighborTotal = 0;
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        neighborTotal++;
        byte neighborPersonality = getPersonality(getLastValueReceivedOnFace(f));
        if (neighborPersonality == INTROVERT) {
          introvertNeighbors++;
        } else if (neighborPersonality = EXTROVERT) {
          extrovertNeighbors++;
        }
      }
    }
    //remember to count yourself
    neighborTotal++;
    if (personality == INTROVERT) {
      introvertNeighbors++;
    } else if (personality == EXTROVERT) {
      extrovertNeighbors++;
    }

    //change personality based on this information
    //how many introverts should there be?
    byte introvertsNeeded = (neighborTotal * INTROVERT_CHANCE) / 100;
    if (introvertsNeeded == 0) {
      introvertsNeeded = 1;
    }

    if (introvertNeighbors > introvertsNeeded) {
      personality = EXTROVERT;
    } else if (introvertNeighbors < introvertsNeeded) {
      personality = INTROVERT;
    }
  }//end neighborhood check

  //listen for the next phase being triggered
  if (phaseTriggered(PLAY) || phaseTimer.isExpired()) {
    phaseTimer.set(PLAY_PHASE_TIME);
    phase = PLAY;
  }
}

void playLoop() {
  //listen for the next phase being triggered, set results
  if (phaseTriggered(RESULT) || phaseTimer.isExpired()) {//time to set the results
    if (isAlone() && personality == INTROVERT) {
      success = true;
    } else if (!isAlone() && personality == EXTROVERT) {
      success = true;
    } else {
      success = false;
    }
    phaseTimer.set(RESULT_PHASE_TIME);
    phase = RESULT;
    //pulseTimer.set(PULSE_DURATION);
  }
}

void resultLoop() {
  //listen for the next phase being triggered
  if (phaseTriggered(WAIT) || phaseTimer.isExpired()) {
    phaseTimer.set(WAIT_PHASE_TIME);
    phase = WAIT;
    //set random personality
    if (random(100) < INTROVERT_CHANCE) {
      personality = INTROVERT;
    } else {
      personality = EXTROVERT;
    }
  }
}

bool phaseTriggered(byte nextPhase) {
  bool triggered = false;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getPhase(getLastValueReceivedOnFace(f)) == nextPhase) {
        triggered = true;
      }
    }
  }
  return triggered;
}

void waitDisplay() {
  if (flickerTimer.isExpired()) {
    FOREACH_FACE(f) {
      //      //byte baseBrightness = 150 - map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, 100);
      //      byte baseBrightness = map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, 150);
      //      setColorOnFace(dim(WHITE, random(50) + random(50) + baseBrightness), f);

      setColorOnFace(dim(WHITE, random(50) + random(50) + 155), f);

      int colorChance = WAIT_PHASE_COLOR_CHANCE - map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, WAIT_PHASE_COLOR_CHANCE);

      if (random(WAIT_PHASE_COLOR_CHANCE) < colorChance) {//increasingly common event
        if (random(1) == 0) {
          setColorOnFace(makeColorHSB(EXTROVERT_HUE, EXTROVERT_SAT, 255), f);
        } else {
          setColorOnFace(makeColorHSB(INTROVERT_HUE, INTROVERT_SAT, 255), f);
        }
      }
    }

    //determine new flicker time
    int flickerTime = map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, FLICKER_DURATION);
    flickerTimer.set(flickerTime);
  }
}


#define SPIN_DURATION 350

void resultDisplay() {

  byte personalitySaturation = EXTROVERT_SAT;
  byte personalityHue = EXTROVERT_HUE;
  if (personality == INTROVERT) {
    personalitySaturation = INTROVERT_SAT;
    personalityHue = INTROVERT_HUE;
  }

  if (success) {
    FOREACH_FACE(f) {
      int cycleOffset = (SPIN_DURATION / 6) * f;
      int cyclePosition = (millis() + cycleOffset) % SPIN_DURATION;
      byte faceSaturation = map(cyclePosition, 0, SPIN_DURATION, 0, personalitySaturation);

      setColorOnFace(makeColorHSB(personalityHue, faceSaturation, 255), f);
    }
  } else {//failure

    setColor(RED);

    //    if (pulseTimer.isExpired()) {
    //      pulseTimer.set(PULSE_DURATION);
    //    }
    //
    //    byte progress = map(pulseTimer.getRemaining(), 0, PULSE_DURATION, 0, 255);
    //    byte progressSin = sin8_C(progress);
    //    byte hueMap = 0;
    //    byte satMap = 0;
    //    if (personality == INTROVERT) {
    //      hueMap = map(progressSin, 0, 255, INTROVERT_HUE, 255);
    //      satMap = map(progressSin, 0, 255, INTROVERT_SAT, 255);
    //    } else {
    //      hueMap = map(progressSin, 0, 255, 0, EXTROVERT_HUE);
    //      satMap = map(progressSin, 0, 255, EXTROVERT_SAT, 255);
    //    }
    //
    //    setColor(makeColorHSB(hueMap, satMap, 255));

  }
}

byte getPhase(byte data) {
  return ((data >> 1) & 3);
}

byte getPersonality(byte data) {
  return (data & 1);
}
