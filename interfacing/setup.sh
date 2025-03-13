# Load the network device
source net_device_discovery.sh

if [[ -z $1 ]]; then
    echo 'Please specify the setup mode: setup / setup_base_image / setup_from_base / setup_from_base_local'
    exit 1
fi

if [[ $1 != 'setup_base_image' && -z $2 ]] ; then
    echo 'Please provide the number of new robots to be setup.'
    exit 1
fi

# Construct the command to run get_ip_by_mac.py
if [[ $1 == 'setup_base_image' ]]; then
    CMD="sudo python3 get_ip_by_mac.py ../config/mac_list.json $INTERFACE $1 -n 1"
else
    CMD="sudo python3 get_ip_by_mac.py ../config/mac_list.json $INTERFACE $1 -n $2"
fi

# Execute the constructed command
eval $CMD