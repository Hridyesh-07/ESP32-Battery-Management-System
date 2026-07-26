#define BLYNK_TEMPLATE_ID "TMPL3b4BKE5ib"
#define BLYNK_TEMPLATE_NAME "Battery Intelligence Engine"
#define BLYNK_AUTH_TOKEN "I-QFSPm9YlXcJsnSoZBZ9b0VueQlSaJe"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
const int RELAY_PIN = 25;
const int BUZZER_PIN = 26;

LiquidCrystal_I2C lcd(0x27, 16, 2);

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ================= PINS =================

const int cellPins[4] = {34, 35, 32, 33};

const float WEAK_CELL_THRESHOLD = 3.0;
const float OVERVOLTAGE_THRESHOLD = 4.15;
const float SENSOR_LOW_LIMIT = 0.05;
const float SENSOR_HIGH_LIMIT = 4.25;
const float FLUCTUATION_THRESHOLD = 0.5;

const unsigned long RECOVERY_DELAY = 3000;
const unsigned long FLUCTUATION_INTERVAL = 1000;

// ================= HEALTH STATES =================

enum HealthState
{
    HEALTHY,
    MINOR_IMBALANCE,
    CRITICAL_IMBALANCE,
    PACK_FAILURE
};

enum SafetyState
{
    SAFE,
    WEAK_CELL_FAULT,
    OVERVOLTAGE_FAULT,
    SENSOR_FAULT,
    FLUCTUATION_FAULT,
    FROZEN_ADC_FAULT,
    RELAY_MISMATCH_FAULT,
    RECOVERY
};

enum RuntimeMode
{
    NORMAL,
    DEGRADED,
    FAILSAFE,
    SHUTDOWN
};

// ================= BATTERY PACK =================

struct BatteryPack
{
    float cell[4];

    float packVoltage;

    float averageVoltage;

    float imbalancePercent;

    int weakestCell;
    int strongestCell;

    float weakestVoltage;
    float strongestVoltage;

    HealthState health;
};

struct FaultLog
{
    String faultName;

    unsigned long timestamp;

    bool recovered;
};

BatteryPack pack;

SafetyState safetyState = SAFE;

RuntimeMode runtimeMode = NORMAL;

bool relayEnabled = true;

bool relayFeedback = true;

bool recoveryActive = false;

unsigned long recoveryStartTime = 0;

unsigned long lastBuzzerToggle = 0;

bool buzzerState = false;

float previousVoltage[4];

bool cellIsolated[4] = {false, false, false, false};

unsigned long lastFluctuationCheck = 0;

float frozenReference[4];

unsigned long frozenStartTime[4];

const unsigned long FROZEN_ADC_TIME = 60000; // 10 seconds

const float FROZEN_TOLERANCE = 0.01;

// ================= TIMERS =================

unsigned long lastPrint = 0;

unsigned long lastScreenChange = 0;

unsigned long lastWiFiRetry = 0;

int currentScreen = 0;

const unsigned long SCREEN_INTERVAL = 2500;

int totalFaults = 0;

FaultLog faultLogs[20];

int faultCount = 0;

int queuedEvents = 0;

bool sensorFaultActive = false;

bool overVoltageFaultActive = false;

bool weakCellFaultActive = false;

bool frozenADCFaultActive = false;

bool relayMismatchFaultActive = false;

bool fluctuationFaultActive = false;

bool fluctuationInitialized = false;

SafetyState previousSafetyState = SAFE;

bool lcdNeedsRefresh = true;

// ================= CLOUD TELEMETRY =================

float lastSentCell[4] = {-1, -1, -1, -1};

float lastSentPackVoltage = -1;

float lastSentAverageVoltage = -1;

float lastSentImbalance = -1;

HealthState lastSentHealth = HEALTHY;

SafetyState lastSentSafety = SAFE;

RuntimeMode lastSentRuntime = NORMAL;

int lastSentTotalFaults = -1;

// =================================================
// SETUP
// =================================================

void detectFaults();

void manageSafety();

void controlRelay();

void controlBuzzer();

void updateLCD();

String getSafetyString();

void updateRuntimeMode();

void isolateCell(int cell);

void clearIsolation();

void sendTelemetry();

void printReport();

void syncQueuedEvents();

void setup()
{
    Serial.begin(115200);

    Serial.print("Connecting to WiFi");

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");

    Serial.println("Connecting to Blynk...");

    Blynk.config(BLYNK_AUTH_TOKEN);

    if (Blynk.connect(10000))
    {
        Serial.println("Blynk Connected!");
    }
    else
    {
        Serial.println("Blynk Connection Failed!");
    }

    lcd.init();
    lcd.backlight();

    Serial.println("=================================");
    Serial.println("Battery Intelligence Engine");
    Serial.println("=================================");
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);

    for(int i=0;i<4;i++)
    {
    previousVoltage[i] = 0;

    frozenReference[i] = 0;

    frozenStartTime[i] = millis();
    }
}

// =================================================
// LOOP
// =================================================

void loop()
{
    
    // WiFi Reconnect Handling

    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastWiFiRetry >= 5000)
        {
            lastWiFiRetry = millis();
            WiFi.begin(ssid, pass);
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if(!Blynk.connected())
        {
            Blynk.connect();
        }

        Blynk.run();

        syncQueuedEvents();
    }
    
    readCells();

    calculatePackVoltage();
    
    calculateAverage();

    findWeakestCell();

    findStrongestCell();

    calculateImbalance();

    evaluateHealth();

    detectFaults();

    updateRuntimeMode();

    manageSafety();

    controlRelay();

    controlBuzzer();

    updateLCD();

    printReport();

    sendTelemetry();

}

// =================================================
// READ CELLS
// =================================================

void readCells()
{
    for (int i = 0; i < 4; i++)
    {
        int adc = analogRead(cellPins[i]);

        // Simulated Lithium Cell (0V - 4.2V)
        pack.cell[i] = (adc / 4095.0) * 4.2;
    }
}

// =================================================
// PACK VOLTAGE
// =================================================

void calculatePackVoltage()
{
    pack.packVoltage = 0;

    for (int i = 0; i < 4; i++)
    {
        pack.packVoltage += pack.cell[i];
    }
}

// =================================================
// PACK AVERAGE
// =================================================

void calculateAverage()
{
    float total = 0;

    for (int i = 0; i < 4; i++)
    {
        total += pack.cell[i];
    }

    pack.averageVoltage = total / 4.0;
}

// =================================================
// WEAKEST CELL
// =================================================

void findWeakestCell()
{
    pack.weakestCell = 0;
    pack.weakestVoltage = pack.cell[0];

    for (int i = 1; i < 4; i++)
    {
        if (pack.cell[i] < pack.weakestVoltage)
        {
            pack.weakestVoltage = pack.cell[i];
            pack.weakestCell = i;
        }
    }
}

// =================================================
// STRONGEST CELL
// =================================================

void findStrongestCell()
{
    pack.strongestCell = 0;
    pack.strongestVoltage = pack.cell[0];

    for (int i = 1; i < 4; i++)
    {
        if (pack.cell[i] > pack.strongestVoltage)
        {
            pack.strongestVoltage = pack.cell[i];
            pack.strongestCell = i;
        }
    }
}

// =================================================
// IMBALANCE CALCULATION
// =================================================

void calculateImbalance()
{
    pack.imbalancePercent =
        ((pack.strongestVoltage -
          pack.weakestVoltage)
         / pack.averageVoltage) *
        100.0;
}

// =================================================
// HEALTH EVALUATION
// =================================================

void evaluateHealth()
{
    const float PACK_FAILURE_VOLTAGE = 2.8;

    if (pack.weakestVoltage < PACK_FAILURE_VOLTAGE)
    {
        pack.health = PACK_FAILURE;
        return;
    }

    if (pack.imbalancePercent < 5.0)
    {
        pack.health = HEALTHY;
    }
    else if (pack.imbalancePercent < 10.0)
    {
        pack.health = MINOR_IMBALANCE;
    }
    else if (pack.imbalancePercent < 20.0)
    {
        pack.health = CRITICAL_IMBALANCE;
    }
    else
    {
        pack.health = PACK_FAILURE;
    }
}

// =================================================
// HEALTH STRING
// =================================================

String getHealthString()
{
    switch (pack.health)
    {
        case HEALTHY:
            return "HEALTHY";

        case MINOR_IMBALANCE:
            return "MINOR IMBALANCE";

        case CRITICAL_IMBALANCE:
            return "CRITICAL IMBALANCE";

        case PACK_FAILURE:
            return "PACK FAILURE";
    }

    return "UNKNOWN";
}

String getRiskLevel()
{
    switch(pack.health)
    {
        case HEALTHY:
            return "LOW";

        case MINOR_IMBALANCE:
            return "MEDIUM";

        case CRITICAL_IMBALANCE:
            return "HIGH";

        case PACK_FAILURE:
            return "CRITICAL";
    }

    return "UNKNOWN";
}

String getRecommendation()
{
    switch(safetyState)
    {
        case SAFE:
            return "Normal Operation";

        case WEAK_CELL_FAULT:
            return "Recharge Cell";

        case OVERVOLTAGE_FAULT:
            return "Disconnect Charger";

        case SENSOR_FAULT:
            return "Inspect Sensor";

        case FLUCTUATION_FAULT:
            return "Check Battery";

        case FROZEN_ADC_FAULT:
            return "Restart Sensor";

        case RELAY_MISMATCH_FAULT:
            return "Inspect Relay";

        default:
            return "Check Battery System";
    }
}

void detectFaults()
{
    // Sensor Error
    for(int i=0;i<4;i++)
    {
        if(pack.cell[i] < SENSOR_LOW_LIMIT ||
           pack.cell[i] > SENSOR_HIGH_LIMIT)
        {
            if(!sensorFaultActive)

            {
              logFault("SENSOR_FAULT");

              sensorFaultActive = true;
            }

isolateCell(i);

safetyState = SENSOR_FAULT;

previousSafetyState = SAFE;

lcdNeedsRefresh = true;

return;
        }
    }

    // Overvoltage
    if(pack.strongestVoltage > OVERVOLTAGE_THRESHOLD)
    {
        if(!overVoltageFaultActive)
        {
          logFault("OVERVOLTAGE_FAULT");

          overVoltageFaultActive = true;

          isolateCell(pack.strongestCell);
        }

safetyState = OVERVOLTAGE_FAULT;

return;
    }

    // Weak Cell
    if(pack.weakestVoltage < WEAK_CELL_THRESHOLD)
    {
        if(!weakCellFaultActive)
        {
          logFault("WEAK_CELL_FAULT");

          weakCellFaultActive = true;
        }

isolateCell(pack.weakestCell);

safetyState = WEAK_CELL_FAULT;

return;
    }
    // Frozen ADC Detection

    for(int i = 0; i < 4; i++)
    {
        if(abs(pack.cell[i] - frozenReference[i]) < FROZEN_TOLERANCE)
        {
            if(millis() - frozenStartTime[i] >= FROZEN_ADC_TIME)
            {
                if(!frozenADCFaultActive)
                {
                    logFault("FROZEN_ADC_FAULT");

                    frozenADCFaultActive = true;
                }

                isolateCell(i);

                safetyState = FROZEN_ADC_FAULT;

                return;
            }
        }
        else
        {
            frozenReference[i] = pack.cell[i];

            frozenStartTime[i] = millis();

            frozenADCFaultActive = false;
        }
    }

    // Relay Mismatch Detection

    relayFeedback = relayEnabled;

    if(relayFeedback != relayEnabled)
    {
        if(!relayMismatchFaultActive)
        {
            logFault("RELAY_MISMATCH_FAULT");

            relayMismatchFaultActive = true;
        }

        safetyState = RELAY_MISMATCH_FAULT;

        return;
    }

    // Initialize fluctuation reference only once

    if(!fluctuationInitialized)
    {
        for(int i = 0; i < 4; i++)
        {
            previousVoltage[i] = pack.cell[i];
        }

        fluctuationInitialized = true;
    }

    // Rapid Fluctuation
    if(millis() - lastFluctuationCheck >= FLUCTUATION_INTERVAL)
    {
        lastFluctuationCheck = millis();

        for(int i = 0; i < 4; i++)
        {
            float diff =
                abs(pack.cell[i] - previousVoltage[i]);

            if(pack.cell[i] > WEAK_CELL_THRESHOLD &&
               pack.cell[i] < OVERVOLTAGE_THRESHOLD)
            {
                if(diff > FLUCTUATION_THRESHOLD)
                {
                    if(!fluctuationFaultActive)
                    {
                      logFault("FLUCTUATION_FAULT");

                      fluctuationFaultActive = true;

                      isolateCell(i);
                    }

safetyState = FLUCTUATION_FAULT;

                    Serial.print("Rapid Fluctuation Cell ");
                    Serial.println(i + 1);

                    previousVoltage[i] = pack.cell[i];

                    return;
                }
            }

            previousVoltage[i] = pack.cell[i];
        }
    }
// No faults detected
sensorFaultActive = false;
overVoltageFaultActive = false;
weakCellFaultActive = false;
fluctuationFaultActive = false;
frozenADCFaultActive = false; 
relayMismatchFaultActive = false;

clearIsolation();

safetyState = SAFE;
}

void updateRuntimeMode()
{
    if(pack.health == PACK_FAILURE)
    {
        runtimeMode = SHUTDOWN;
    }

    else if(safetyState == SENSOR_FAULT ||
            safetyState == OVERVOLTAGE_FAULT ||
            safetyState == FROZEN_ADC_FAULT ||
            safetyState == RELAY_MISMATCH_FAULT)
    {
        runtimeMode = FAILSAFE;
    }

    else if(safetyState == WEAK_CELL_FAULT ||
            safetyState == FLUCTUATION_FAULT)
    {
        runtimeMode = DEGRADED;
    }

    else
    {
        runtimeMode = NORMAL;
    }
}

// =================================================
// CELL ISOLATION
// =================================================

void isolateCell(int cell)
{
    if(cell >= 0 && cell < 4)
    {
        cellIsolated[cell] = true;
    }
}

void clearIsolation()
{
    for(int i = 0; i < 4; i++)
    {
        cellIsolated[i] = false;
    }
}

void manageSafety()
{
    if(safetyState != SAFE)
    {
        relayEnabled = false;

        recoveryActive = false;

        return;
    }

    if(!relayEnabled)
    {
        if(!recoveryActive)
        {
            recoveryStartTime = millis();

            recoveryActive = true;
        }

        if(millis() - recoveryStartTime >= RECOVERY_DELAY)
        {
            relayEnabled = true;

            recoveryActive = false;
        }
    }
}

void controlRelay()
{
    digitalWrite(RELAY_PIN,
                 relayEnabled ? HIGH : LOW);
}

void controlBuzzer()
{
    if(safetyState == SAFE)
    {
        digitalWrite(BUZZER_PIN, LOW);

        buzzerState = false;

        return;
    }

    if(millis() - lastBuzzerToggle >= 500)
    {
        lastBuzzerToggle = millis();

        buzzerState = !buzzerState;

        digitalWrite(BUZZER_PIN,
                     buzzerState);
    }
}

void updateLCD()
{
    // =================================================
    // FAULT OVERRIDE MODE
    // =================================================

    if (safetyState != SAFE)
    {
        if(previousSafetyState != safetyState || lcdNeedsRefresh)
        {
            lcd.clear();

            previousSafetyState = safetyState;

            lcdNeedsRefresh = false;
        }

        switch(safetyState)
        {
            case WEAK_CELL_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("WEAK CELL");

                lcd.setCursor(0, 1);
                lcd.print("RELAY OFF");
                break;

            case OVERVOLTAGE_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("OVERVOLTAGE");

                lcd.setCursor(0, 1);
                lcd.print("RELAY OFF");
                break;

            case SENSOR_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("SENSOR ERROR");

                lcd.setCursor(0, 1);
                lcd.print("RELAY OFF");
                break;

            case FLUCTUATION_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("UNSTABLE CELL");

                lcd.setCursor(0, 1);
                lcd.print("RELAY OFF");
                break;
            
            case FROZEN_ADC_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("FROZEN ADC");

                lcd.setCursor(0, 1);
                lcd.print("RELAY OFF");
                break;
            
            case RELAY_MISMATCH_FAULT:

                lcd.setCursor(0, 0);
                lcd.print("RELAY ERROR");

                lcd.setCursor(0, 1);
                lcd.print("CHECK RELAY");
                break;

            default:
                break;
        }

        return;
    }

    // =================================================
    // SCREEN ROTATION
    // =================================================

    if (millis() - lastScreenChange >= SCREEN_INTERVAL)
    {
        lastScreenChange = millis();

        currentScreen++;

        if (currentScreen > 4)
        {
            currentScreen = 0;
        }

        lcd.clear();
    }

    // =================================================
    // SCREEN 1
    // =================================================

    if (currentScreen == 0)
    {
        lcd.setCursor(0, 0);
        lcd.print("C1:");
        lcd.print(pack.cell[0], 1);

        lcd.setCursor(8, 0);
        lcd.print("C2:");
        lcd.print(pack.cell[1], 1);

        lcd.setCursor(0, 1);
        lcd.print("C3:");
        lcd.print(pack.cell[2], 1);

        lcd.setCursor(8, 1);
        lcd.print("C4:");
        lcd.print(pack.cell[3], 1);
    }

    // =================================================
    // SCREEN 2
    // =================================================

    else if (currentScreen == 1)
    {
        lcd.setCursor(0, 0);
        lcd.print("AVG:");
        lcd.print(pack.averageVoltage, 2);

        lcd.setCursor(0, 1);
        lcd.print("IMB:");
        lcd.print(pack.imbalancePercent, 1);
        lcd.print("% ");
    }

    // =================================================
    // SCREEN 3
    // =================================================

    else if (currentScreen == 2)
    {
        lcd.setCursor(0, 0);

        switch (pack.health)
        {
            case HEALTHY:
                lcd.print("HEALTHY");
                break;

            case MINOR_IMBALANCE:
                lcd.print("MINOR IMB");
                break;

            case CRITICAL_IMBALANCE:
                lcd.print("CRITICAL IMB");
                break;

            case PACK_FAILURE:
                lcd.print("PACK FAILURE");
                break;
        }

        lcd.setCursor(0, 1);
        lcd.print("PACK:");
        lcd.print(pack.packVoltage, 1);
        lcd.print("V");
    }

    // =================================================
    // SCREEN 4
    // =================================================

    else if (currentScreen == 3)
    {
        lcd.setCursor(0, 0);
        lcd.print("SAFE");

        lcd.setCursor(0, 1);

        if (relayEnabled)
        {
            lcd.print("RELAY:ON ");
        }
        else
        {
            lcd.print("RELAY:OFF");
        }
    }

    // =================================================
    // SCREEN 5
    // =================================================

    else if (currentScreen == 4)
    {
        lcd.setCursor(0, 0);
        lcd.print("W:C");
        lcd.print(pack.weakestCell + 1);

        lcd.print(" S:C");
        lcd.print(pack.strongestCell + 1);

        lcd.setCursor(0, 1);
        lcd.print("FAULTS:");
        lcd.print(totalFaults);
    }
}

void logFault(String faultName)
{
    if(faultCount >= 20)
        return;

    faultLogs[faultCount].faultName = faultName;

    faultLogs[faultCount].timestamp = millis();

    faultLogs[faultCount].recovered = false;

    faultCount++;

    totalFaults++;

    queuedEvents++;

    Serial.print("FAULT LOGGED: ");
    Serial.print(faultName);

    Serial.print(" Time: ");

    Serial.println(millis());

    Serial.print("Fault Count = ");
    Serial.println(faultCount);

    Serial.print("Total Faults = ");
    Serial.println(totalFaults);
}

// =================================================
// BLYNK TELEMETRY
// =================================================

void sendTelemetry()
{
    if (!Blynk.connected())
        return;
    
    bool shouldSend = false;

    // Cell voltage changed?
    for(int i = 0; i < 4; i++)
    {
        if(abs(pack.cell[i] - lastSentCell[i]) > 0.05)
        {
            shouldSend = true;
            break;
        }
    }

    // Pack values changed?
    if(abs(pack.packVoltage - lastSentPackVoltage) > 0.1)
        shouldSend = true;

    if(abs(pack.averageVoltage - lastSentAverageVoltage) > 0.05)
        shouldSend = true;

    if(abs(pack.imbalancePercent - lastSentImbalance) > 1.0)
        shouldSend = true;

    // Health changed?
    if(pack.health != lastSentHealth)
        shouldSend = true;

    // Safety changed?
    if(safetyState != lastSentSafety)
        shouldSend = true;

    // Runtime changed?
    if(runtimeMode != lastSentRuntime)
        shouldSend = true;

    // Fault count changed?
    if(totalFaults != lastSentTotalFaults)
        shouldSend = true;

    // Nothing changed
    if(!shouldSend)
        return;

    Blynk.virtualWrite(V0, pack.cell[0]);
    Blynk.virtualWrite(V1, pack.cell[1]);
    Blynk.virtualWrite(V2, pack.cell[2]);
    Blynk.virtualWrite(V3, pack.cell[3]);

    Blynk.virtualWrite(V4, pack.packVoltage);
    Blynk.virtualWrite(V5, pack.averageVoltage);
    Blynk.virtualWrite(V6, pack.imbalancePercent);

    Blynk.virtualWrite(V7, getHealthString());

    switch (runtimeMode)
    {
        case NORMAL:
            Blynk.virtualWrite(V8, "NORMAL");
            break;

        case DEGRADED:
            Blynk.virtualWrite(V8, "DEGRADED");
            break;

        case FAILSAFE:
            Blynk.virtualWrite(V8, "FAILSAFE");
            break;

        case SHUTDOWN:
            Blynk.virtualWrite(V8, "SHUTDOWN");
            break;
    }

    switch (safetyState)
    {
        case SAFE:
            Blynk.virtualWrite(V9, "SAFE");
            break;

        case WEAK_CELL_FAULT:
            Blynk.virtualWrite(V9, "WEAK CELL");
            break;

        case OVERVOLTAGE_FAULT:
            Blynk.virtualWrite(V9, "OVERVOLTAGE");
            break;

        case SENSOR_FAULT:
            Blynk.virtualWrite(V9, "SENSOR");
            break;

        case FLUCTUATION_FAULT:
            Blynk.virtualWrite(V9, "FLUCTUATION");
            break;

        case FROZEN_ADC_FAULT:
            Blynk.virtualWrite(V9, "FROZEN ADC");
            break;

        case RELAY_MISMATCH_FAULT:
            Blynk.virtualWrite(V9, "RELAY ERROR");
            break;

        default:
            Blynk.virtualWrite(V9, "UNKNOWN");
            break;
    }
    // Save latest transmitted values

    for(int i = 0; i < 4; i++)
    {
        lastSentCell[i] = pack.cell[i];
    }

    Blynk.virtualWrite(V13, WiFi.RSSI());

    Blynk.virtualWrite(V10, totalFaults);

    lastSentTotalFaults = totalFaults;

    if (faultCount > 0)
    {
        Blynk.virtualWrite(V12, faultLogs[faultCount - 1].faultName);
    }
    else
    {
        Blynk.virtualWrite(V12, "NONE");
    }

    Blynk.virtualWrite(V11, getRiskLevel());

    Blynk.virtualWrite(V14, getRecommendation());

    lastSentPackVoltage = pack.packVoltage;

    lastSentAverageVoltage = pack.averageVoltage;

    lastSentImbalance = pack.imbalancePercent;

    lastSentHealth = pack.health;

    lastSentSafety = safetyState;

    lastSentRuntime = runtimeMode;
}

void syncQueuedEvents()
{
    if (!Blynk.connected())
        return;

    if (queuedEvents == 0)
        return;

    for (int i = 0; i < faultCount; i++)
    {
        if (!faultLogs[i].recovered)
        {
            String eventName;

            if(faultLogs[i].faultName == "WEAK_CELL_FAULT")
                eventName = "weak_cell";

            else if(faultLogs[i].faultName == "OVERVOLTAGE_FAULT")
                eventName = "over_voltage";

            else if(faultLogs[i].faultName == "SENSOR_FAULT")
                eventName = "sensor_fault";

            else if(faultLogs[i].faultName == "FLUCTUATION_FAULT")
                eventName = "fluctuation_fault";

            else if(faultLogs[i].faultName == "FROZEN_ADC_FAULT")
                eventName = "frozen_adc";

            else if(faultLogs[i].faultName == "RELAY_MISMATCH_FAULT")
                eventName = "relay_fault";

            else
                continue;

            Blynk.logEvent(eventName.c_str(), faultLogs[i].faultName);

            faultLogs[i].recovered = true;
        }
    }

queuedEvents = 0;
}

// =================================================
// SERIAL REPORT
// =================================================

void printReport()
{
    if (millis() - lastPrint < 1000)
        return;

    lastPrint = millis();

    Serial.println("\n=================================");

    for (int i = 0; i < 4; i++)
    {
        Serial.print("Cell ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(pack.cell[i], 2);
        Serial.println(" V");
    }

    Serial.println();

    Serial.print("Pack Voltage: ");
    Serial.print(pack.packVoltage, 2);
    Serial.println(" V");

    Serial.print("Pack Average: ");
    Serial.print(pack.averageVoltage, 2);
    Serial.println(" V");

    Serial.print("Weakest Cell: C");
    Serial.print(pack.weakestCell + 1);
    Serial.print(" (");
    Serial.print(pack.weakestVoltage, 2);
    Serial.println(" V)");

    Serial.print("Strongest Cell: C");
    Serial.print(pack.strongestCell + 1);
    Serial.print(" (");
    Serial.print(pack.strongestVoltage, 2);
    Serial.println(" V)");

    Serial.print("Imbalance: ");
    Serial.print(pack.imbalancePercent, 2);
    Serial.println("%");

    Serial.print("Health: ");
    Serial.println(getHealthString());

    Serial.print("Safety State: ");

    switch(safetyState)
    {
        case SAFE:
            Serial.println("SAFE");
            break;

        case WEAK_CELL_FAULT:
            Serial.println("WEAK_CELL_FAULT");
            break;

        case OVERVOLTAGE_FAULT:
            Serial.println("OVERVOLTAGE_FAULT");
            break;

        case SENSOR_FAULT:
            Serial.println("SENSOR_FAULT");
            break;

        case FLUCTUATION_FAULT:
            Serial.println("FLUCTUATION_FAULT");
            break;

        case FROZEN_ADC_FAULT:
            Serial.println("FROZEN_ADC_FAULT");
            break;

        case RELAY_MISMATCH_FAULT:
            Serial.println("RELAY_MISMATCH_FAULT");
            break;

        default:
            Serial.println("UNKNOWN");
            break;
    }

    Serial.print("Runtime Mode: ");

    switch(runtimeMode)
    {
        case NORMAL:
            Serial.println("NORMAL");
            break;

        case DEGRADED:
            Serial.println("DEGRADED");
            break;

        case FAILSAFE:
            Serial.println("FAILSAFE");
            break;

        case SHUTDOWN:
            Serial.println("SHUTDOWN");
            break;
    }

    Serial.println();

    Serial.print("Isolated Cells: ");

    bool isolatedFound = false;

    for(int i = 0; i < 4; i++)
    {
        if(cellIsolated[i])
        {
            Serial.print("C");
            Serial.print(i + 1);
            Serial.print(" ");

            isolatedFound = true;
        }
    }

    if(!isolatedFound)
    {
        Serial.print("NONE");
    }

    Serial.println();

    Serial.println("=================================");
}