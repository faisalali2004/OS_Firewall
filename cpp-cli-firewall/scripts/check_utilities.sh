#!/bin/bash

# List of required utilities
required_utilities=("gcc" "g++" "make" "cmake")

# Function to check if a utility is installed
check_utility() {
    command -v "$1" >/dev/null 2>&1
}

# Check for each required utility
for utility in "${required_utilities[@]}"; do
    if ! check_utility "$utility"; then
        echo "Error: Required utility '$utility' is not installed."
        echo "Please install it and try again."
        exit 1
    fi
done

echo "All required utilities are installed."