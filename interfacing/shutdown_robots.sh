if [[ $1 -eq 0 ]] ; then
    echo 'Please provide the number of working robots on the testbed.'
    exit 0
fi

sudo python3 get_ip_by_mac.py ../config/mac_list.json enp3s0 -c "sudo shutdown now" -n $1
