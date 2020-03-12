enum phases {WAIT, PLAY, RESULT};
enum personalities {INTROVERT, EXTROVERT};

byte phase = WAIT;
byte personality = INTROVERT;
bool success = false;

Timer phaseTimer;
#define WAIT_PHASE_TIME 3000
#define PLAY_PHASE_TIME 4000
#define RESULT_PHASE_TIME 5000

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
      setColor(dim(WHITE, 100));
      break;
    case PLAY:
      setColor(CYAN);
      if (personality == INTROVERT) {
        setColor(BLUE);
      } else {//EXTROVERT
        setColor(MAGENTA);
      }
      break;
    case RESULT:
      if (success == true) {
        setColor(WHITE);
      } else {
        setColor(RED);
      }
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

byte getPhase(byte data) {
  return ((data > 1) & 3);
}

byte getPersonality(byte data) {
  return (data & 1);
}
