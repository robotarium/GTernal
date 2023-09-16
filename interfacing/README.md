# Interfacing Scripts

Make sure to change the network device name (ex. enp3s0) in "restart_docker_containers.sh" and "shutdown_robots.sh" to the name of the network device of the computer.

To check the network device name of the computer (assuming the computer is connected to internet via a router), run
```
ifconfig
```
and look for a device with an 'inet' ip address with 192.168.1.x.

For "shutdown_robots.sh," sshpass needs to be installed. It can be installed with
```
sudo apt-get install sshpass
```
