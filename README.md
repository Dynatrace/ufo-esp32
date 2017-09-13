# Dynatrace UFO
The 2nd generation of the Dynatrace UFO runs now with ESP32 microcontroller [(specs)](doc/SPECS.md). Also, since not everyone has the time print, solder and assemble themselves, we have produced a limited batch of ufos for your convienience. [https://dynatrace.com/ufo
![](ufofeatures.png)](https://dynatrace.com/ufo)

# How can i foster a DevOps culture with the Dynatrace UFO?
## See the status of your CI and DevOps process in real-time
For companies that face the challenges of a distributed team, the Dynatrace DevOps UFO is a highly visible IoT gadget to share the status of a project. Using our UFOs, a quick stroll through the office, or a glance to the other side of the room, can alert you to problems.
## Improve quality and protect digital performance
The UFO monitors progress at every stage of the continuous delivery process, with a separate device for each team or feature. If there are issues that impact getting a build completed, such as code that doesn’t compile, the LEDs turn red.


[![ufo builds devops culture](ufobuildsdevopsculture.jpg) https://www.youtube.com/watch?v=6z9BTHhvWSU](https://www.youtube.com/watch?v=6z9BTHhvWSU)

## Any other visualization need...
Since the UFO has an open REST interface you can use it freely for any other visualization need. For the nerds amongst you, the UFO is also open source. So at your own risk, you are free to hack the UFO for any of your own visualization needs. 

# How do I use the UFO?
## Hanging - Cafeteria, Hallway use.....
![ufo hanging](ufohanging.jpg)
Note: Assembly of the UFO (in particular mounting the device on the ceiling) may only be carried out by qualified personnel. 

## Desktop - Office desk, ...
![ufo on desktop](ufodesktop.jpg)
* mount the 3 rubber feet that came in the box with the UFO
* slide the stabilizing tube along the cable towards the USB plug, so you can bend the cable properly

# Configuring the UFO

## Wifi
When you first power the UFO or every time you press the button on the top, the UFO starts up in Ad-Hoc mode which is symbolized by blue flashing led rings. The SSID is "Ufo" and the IP 192.168.4.1. Use a notebook, tablet or phone to connect to the "Ufo" net and open "http://192.168.4.1/" in a browser. This will open up the web-interface of the UFO where you now can configure your Wifi setting. After applying the settings the UFO will restart and try to connect to the configured Wifi, which is symbolized by yellow flashing led rings. Once a connection has been made the led display starts to display the retrieved (DHCP) IP address. If this does not succeed switch back to Ad-Hoc mode and check the settings.
If the connections succeeds there are several options to connect to the UFO:
* Option 1: Open your browser, make sure its connected to the same Wifi as the UFO. Goto <a href="http://ufo">http://ufo</a>
* Option 2: The rings visualizes the current IP address digit by digit. 192.168... will light 1 led then 9 (5+4)
						then 2 and so on. A dot is visualized as 3 white leds. The individuL digits are separated by a short white flash. The
						IP is visualized over and over again until the first api rest call is issued. To stay in sync every IP address visualization
						run uses a different color.
* Option 3: After a successful connection to the Wifi switch back to Access Point mode (push the button) and look at the
						info section for the latest IP adress retrieved by DHCP.

## API

# Firmware

## Update

## Nerd Zone
[Firmware build instructions](doc/BUILD.md)


# Legal Stuff

Dear Customer,


Please read the operating instructions carefully before putting into operation for the first time.

* The UFO device may only be operated with a USB power adapter with a voltage of 5V and at least a 2A DC. Only use power adapters that are legally approved for the respective country.

* Connecting the UFO device to your PC or laptop requires a special development environment, which is permitted only to trained developers. The user is liable for any possible damage to the PC or UFO.

* Avoid using USB extension cables, as this may lead to the UFO being damaged or displaying unwanted behaviour.

* Assembly of the UFO (in particular mounting the device on the ceiling) may only be carried out by qualified personnel.

* The UFO is to be used exclusively in interior spaces.

### WARRANTY:

1.1. Limited Warranty. Dynatrace warrants to Buyer that, for a period of 90 days from the date of shipment of the Goods, such Goods will materially conform to Dynatrace’s published specifications in effect as of the date of manufacture.

1.2. Disclaimer. EXCEPT FOR THE WARRANTY SET FORTH IN SECTION 1.1 ABOVE, DYNATRACE MAKES NO WARRANTY WHATSOEVER WITH RESPECT TO THE GOODS, INCLUDING ANY (A) WARRANTY OF MERCHANTABILITY; (B) WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE; (C) WARRANTY OF TITLE; OR (D) WARRANTY AGAINST INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY, WHETHER EXPRESS OR IMPLIED BY LAW, COURSE OF DEALING, COURSE OF PERFORMANCE, USAGE OF TRADE OR OTHERWISE.




