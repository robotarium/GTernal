# Load environment variables
source ../config/env_variables.sh

# Automatically get the network device name with IP starting with 192. or 10.
if command -v ip > /dev/null; then
    # For Linux systems
    INTERFACES=$(ip -o -4 addr list | awk '{print $2, $4}' | grep -E '192\.|10\.' | awk '{print $1}' | head -n 1)
elif command -v ifconfig > /dev/null; then
    # For macOS and older Linux systems
    INTERFACES=$(ifconfig | awk '/inet / && $2 ~ /^('$STARTING_IP_ADDRESSES')/ {sub(":", "", iface); print iface} /flags/ {iface=$1}'
)
fi

# Check if any interfaces were found
if [[ -z $INTERFACES ]]; then
    echo "No network interface with IP starting with $STARTING_IP_ADDRESSES found. Please select one manually by typing the number:"
    # Get a list of all network interfaces
    INTERFACES=$(ifconfig | awk '/flags/ {sub(":", "", $1); print $1}')
    select INTERFACE in $INTERFACES; do
        if [[ -n $INTERFACE ]]; then
            break
        else
            echo "Invalid selection. Please try again."
        fi
    done
# Prompt the user to select a network interface if multiple are found
elif [[ $(echo "$INTERFACES" | wc -l) -gt 1 ]]; then
    echo "Multiple network interfaces found. Please select one manually by typing the number:"
    select INTERFACE in $INTERFACES; do
        if [[ -n $INTERFACE ]]; then
            break
        else
            echo "Invalid selection. Please try again."
        fi
    done
else
    INTERFACE=$INTERFACES
fi

echo "Selected Network Interface: $INTERFACE"