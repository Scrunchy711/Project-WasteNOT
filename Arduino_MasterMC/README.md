# Master Microcontroller
Master_FSM contains the main code that communicates the the Sensor Platform Arduino Microcontroller and Sorting System Arduino Microcontroller as slave devices. The MASTER MC runs a Finite State Machine that ensures strict operation process flows between the 2 slave devices so that sensing and sorting processes can be done smoothly and autonomously. The Master MC also controls WS2812B LEDs, which are mounted on the front panel of WasteNOT machine to act as feedback indicators for user interaction on 1) Operation status of Sensor Platform and Sorting system and 2) Full status of our storage system.

# Logic of FSM


