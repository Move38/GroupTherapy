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
#define FLICKER_DURATION 50

void setup() {
  // put your setup code here, to run once:
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
  //do game of life
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
  }
}

void resultLoop() {
  //listen for the next phase being triggered
  if (phaseTriggered(WAIT) || phaseTimer.isExpired()) {
    phaseTimer.set(WAIT_PHASE_TIME);
    phase = WAIT;
    //set random personality
    if (random(1) == 1) {
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
      //byte baseBrightness = 150 - map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, 100);
      byte baseBrightness = map(phaseTimer.getRemaining(), 0, WAIT_PHASE_TIME, 0, 150);
      setColorOnFace(dim(WHITE, random(50) + random(50) + baseBrightness), f);

      if (random(20) == 0) {//rare event
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
    byte redFace = (millis() % SPIN_DURATION) / (SPIN_DURATION / 6);
    setColor(makeColorHSB(personalityHue, personalitySaturation, 255));
    setColorOnFace(RED, redFace);
    setColorOnFace(RED, (redFace + 1) % 6);
    setColorOnFace(RED, (redFace + 2) % 6);
  }
}

byte getPhase(byte data) {
  return ((data >> 1) & 3);
}

byte getPersonality(byte data) {
  return (data & 1);
}
