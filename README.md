# Bandsaw_Control
This code was used with an arduino nano every,an ads 1115 analog to digital converter, and a nextion 3.5" display.  The design of the system allows for the measurment of the band speed, and the descent rate of the band from a linear potentiometer that is fixed to the hydrolic feed of a horizontal bandsaw.  Then using user inputs about the blade type, material type and geometry, the ideal feed and speed will be calculated and displayed.  The operator only has to modify the hydrolic feed to match the calculated feed to get the best material removal rate and blade life.

Some parameters will have to be modified based on the individual bandsaw such as the kinematics equation.

If the bandsaw is belt driven with fixed pully ratio, then some of the code can be modified.  This was designed for a bandsaw that has a variable belt speed, specificly the Jet EHB-8VS

the *.tft file is the file that can be directly loaded on to the Nextion display.  The *.hmi file can be opened and modified within the Nextion editor to modify as you wish.  The *.ino file can be loaded into an IDE that works with arduino such as the arduino ide or platform Io.

TO DO:
1) Some general bugs and working on keeping files global in the Nextion editor, sometimes the entries need to be reentered which is an annoyance
2) Add a PID controller with a stepper to control the hydrolic knob so the MCU can automaticly adjust the dampner to maintain the correct feedrate
3) Add more blades and blade types
4) Add more material geometries and geometry types

References:
1) All feeds/speed/material SFM data came from either the Lennox bandsaw blade guides, or the Machinst Handbook
