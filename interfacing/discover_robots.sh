# Load the network device
source net_device_discovery.sh

sudo python3 get_ip_by_mac.py ../config/mac_list.json $INTERFACE discover