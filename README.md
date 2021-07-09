STM32_self_balancing_robot_GUI

Execute this command in terminal, before running the app and trying to connect:
sudo rfcomm bind hci0 20:17:01:11:67:69
where 20:17:01:11:67:69 is the MAC address of your device
'bind' command waits for connection to be established and passes data from hci0 to rfcomm
'connect' command establishes connection immediately
the connection seems to be more stable and reliable using the 'bind' command

After that run application as administrator and connect to target. (Running app as non admin will refuse to connect)
 - To setup the connection click on 'Settings' -> 'Configure' -> as serial port write 'rfcomm0'
 - After settings go to 'Actions' and click on 'Connect'
