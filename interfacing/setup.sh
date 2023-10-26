if [[ $1 -eq 0 ]] ; then
    echo 'Please provide the number of new robots to be setup.'
    exit 0
fi

sudo python3 get_ip_by_mac.py ../config/mac_list.json en0 scp -f "../setup/setup" -d "/home/pi" -n $1