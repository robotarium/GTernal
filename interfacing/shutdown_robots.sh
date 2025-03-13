# Load the network device
source net_device_discovery.sh

if [[ $1 -eq 0 ]] ; then
    echo 'Please provide the number of working robots on the testbed.'
    exit 0
fi

if [[ -z $2 ]] ; then
    sudo python3 get_ip_by_mac.py ../config/mac_list.json $INTERFACE ssh -c "sudo shutdown now" -n $1
else
    sudo python3 get_ip_by_mac.py ../config/mac_list.json $INTERFACE ssh -c "sudo shutdown now" -n $1 -id $2
fi